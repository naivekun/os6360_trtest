require_relative 'lib/msign/version'

Gem::Specification.new do |s|
  s.name        = 'msign'
  s.version     = MSign::VERSION
  s.date        = '2017-03-21'
  s.author      = 'Cisco Systems, Inc'
  s.summary     = 'Meraki Sign'
  s.description = 'Meraki FIT image signing tool'
  s.files       = Dir.glob("ext/**/*.{c,h}") +
                  Dir.glob("lib/**/*.rb")
  s.extensions  = 'ext/uboot/extconf.rb'
  s.bindir      = 'exe'
  s.executable  = 'msign'
end
