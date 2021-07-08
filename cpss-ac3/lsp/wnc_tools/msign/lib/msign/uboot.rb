require 'base64'
require 'json'
require 'openssl'

require_relative '../../ext/uboot/uboot'

module MSign
  class Uboot

    def initialize(options)
      @keydir = options['keydir']
      @algo   = options['algo']
      @board  = options['board']
      @pid    = options['pid']
      @ticket = options['ticket']
      @release = options['release']
    end

    # 'ext/uboot/uboot' defines the methods:
    #
    # def sign(file)
    # def verify(file)
    # def hash(file)
    #
    # look there for implementation details

    private
    # convert uboot regions list into hash
    def self.hash_regions(algo, regions)
      digest = OpenSSL::Digest.new(algo)
      regions.each { |r| digest.update(r) }
      digest.digest
    end

    # digest and sign
    def sign_regions(algo, keyname, regions)
      keyname = keyname.rpartition('-').first
      hash = Uboot.hash_regions(algo, regions)

      if @keydir
        keyfile = File.expand_path("#{@keydir}/#{keyname}.key")
        privkey = OpenSSL::PKey.read(File.binread(keyfile))
        raise "Not an ECDSA key" unless privkey.respond_to? :dsa_sign_asn1
        privkey.dsa_sign_asn1(hash)
      else
        command = [ 'swims_client',
                    'abraxas', 'signHash',
                    '-product', @board,
                    '-pid', @pid,
                    '-keyName', keyname,
                    '-keyType', @release ? 'RELEASE' : 'DEV',
                    '-notes', 'msign',
                    '-hash', hash.unpack('H*').first,
                    '-algorithm', algo.upcase,
                    '-authType', 'Ticket',
                    '-ticket', @ticket,
        ]
        result = `#{command.join(' ')}`
        unless $?.success?
          puts "Error: command failed (#{$?})"
          puts result
          return nil
        end
        result = JSON.parse(result)
        sig = Base64.decode64(result['signature'])

        # the downloaded signature should look like this for ecdsa sigs:
        #
        # Signature  ::=  SEQUENCE  {
        #     algorithm         OBJECT IDENTIFIER,
        #     signatureValue    BIT STRING  }
        #
        # which is, as far as I can tell, non-standard but seems to be loosely
        # based off of the Certificate struct defined in rfc5280#section-4.1.
        begin
          asn1 = OpenSSL::ASN1.decode(sig)
          asn1 = asn1.value
          asn1 = asn1[1]
          asn1.value
        rescue OpenSSL::ASN1::ASN1Error
          sig
        end
      end

    end

    # digest and verify
    def verify_regions(algo, keyname, sig, regions)
      keyname = keyname.rpartition('-').first
      hash = Uboot.hash_regions(algo, regions)

      if @keydir
        keyfile = File.expand_path("#{@keydir}/#{keyname}.pub")
        pubkey = File.binread(keyfile)
      else
        command = [ 'swims_client',
                    'abraxas', 'fetchPublicKey',
                    '-product', @board,
                    '-keyName', keyname,
                    '-keyType', @release ? 'RELEASE' : 'DEV',
        ]
        result = `#{command.join(' ')}`
        unless $?.success?
          puts "Error: command failed (#{$?})"
          puts result
          return nil
        end
        result = JSON.parse(result)
        pubkey = Base64.decode64(result['publicKey'])
      end

      pubkey = OpenSSL::PKey.read(pubkey)
      raise "Not an ECDSA key" unless pubkey.respond_to? :dsa_verify_asn1
      pubkey.dsa_verify_asn1(hash, sig)
    end

  end
end
