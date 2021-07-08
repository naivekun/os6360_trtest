require 'optparse'

require_relative 'uboot'

module MSign
  TMPFILE = "/tmp/msign.#{Process.pid}"

  module CLI

    def self.expand_path(file)
      file = File.expand_path(file)
      abort "No such file '#{file}'" unless File.exists? file
      file
    end

    def self.sign(file, options)
      file = expand_path(file)
      `cp #{file} #{TMPFILE}`

      begin
        Uboot.new(options).sign(TMPFILE)
        `mv #{TMPFILE} #{file}`
      ensure
        `rm -f #{TMPFILE}`
      end
    end

    def self.verify(file, options)
      file = expand_path(file)

      Uboot.new(options).verify(file)
    end

    def self.hash(file, options)
      file = expand_path(file)
      `cp #{file} #{TMPFILE}`

      begin
        Uboot.new(options).hash(TMPFILE)
        `mv #{TMPFILE} #{file}`
      ensure
        `rm -f #{TMPFILE}`
      end
    end

    def self.resign(file, options)
      file = expand_path(file)
      key = options.fetch('name') { abort "no new key supplied" }

      # change the key-name-hint in all the configurations
      configs = `fdtget -l #{file} /configurations`.split("\n")
      configs.each do |c|
        `fdtput -t s #{file} /configurations/#{c}/signature@1 key-name-hint #{key}`
      end
      sign(file)
    end

    def self.start(argv)
      options = {}
      OptionParser.new do |opts|
        opts.banner = "Usage: script <command> <file> [options]"
        opts.separator ""
        opts.separator "Commands:"
        opts.separator "sign FILE - sign an .itb file"
        opts.separator "verify FILE - verify an .itb file"
        opts.separator "hash FILE - add hash nodes to an .itb file"
        opts.separator "resign FILE KEYNAME - re-sign an .itb file"
        opts.separator ""
        opts.separator "Options:"
        opts.on("-bBOARD",  "--board=BOARD",   "Name of the Meraki board")           { |v| options['board'] = v }
        opts.on("-kKEYDIR", "--keydir=KEYDIR", "Directory containing local keypair") { |v| options['keydir'] = v }
        opts.on("-r",       "--release",       "Sign using release keys")            { |v| options['release'] = v }
        opts.on("-v",       "--verbose",       "Prints extra debug info")            { |v| options['verbose'] = v }
        opts.on("-aALGO",   "--algo=ALGO",     "Hash algo to add")                   { |v| options['algo'] = v }
        opts.on("-nKEY",    "--name=KEY",      "Key to use for re-sign")             { |v| options['name'] = v }
        opts.on("-pPID",    "--pid=PID",       "Name of Cisco product ID")           { |v| options['pid'] = v }
        opts.on("-tTICKET", "--ticket=TICKET", "Ticket to use for signing")          { |v| options['ticket'] = v }
      end.parse! argv

      command = argv.shift
      file = argv.shift

      case command
      when 'sign'
        sign(file, options)
      when 'verify'
        verify(file, options)
      when 'hash'
        hash(file, options)
      when 'resign'
        resign(file, options)
      else
        abort "Not a valid command"
      end
    end

  end
end
