# Ruby-Termios

This extension library provides Termios module.  It enables you to use
termios(3) interface.

Termios module is simple wrapper of termios(3).  It can be included into
IO-family classes and can extend IO-family objects.  In addition, the
methods can use as module function.

## Installation

### Gem

Add this line to your application's Gemfile:

```ruby
gem 'ruby-termios', require: 'termios'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install ruby-termios

### Make

    $ ruby ext/extconf.rb
    $ make install

## Development

After checking out the repo, run `bin/setup` to install dependencies. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/arika/ruby-termios.


## License

The gem is available as open source under the terms of the [Ruby's Licens](https://www.ruby-lang.org/en/about/license.txt).
