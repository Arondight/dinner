#!/usr/bin/env perl

package main;

use warnings;
use strict;
use 5.010;

use CGI qw/:standard :html3/;

use constant {
  IMAGE_URL => '/image.gif',
};

my @rows;

sub showParam {
  @rows = th ['Key', 'Value'];
  push @rows, td [$_, multi_param $_]
    for sort &multi_param;
  print table { -border => undef, -width => '50%' },
              caption b 'Parameters for CGI are:',
              Tr \@rows;
}

sub showEnv {
  @rows = th ['Variables', 'Value'];
  push @rows, td [$_, $ENV{$_}]
    for sort keys %ENV;
  print table { -border => undef, -width => '50%' },
              caption b 'Environment for CGI are:',
              Tr \@rows;

}

print header ('text/html'),
      start_html (-title => "CGI Tests");

print hr,
      img { -src => (+IMAGE_URL) };

print hr;
if ($CGI::VERSION >= 4.08) {
  showParam;
} else {
  print 'Your CGI.pm is too old to work, visit ',
        a ({ -href=>'https://metacpan.org/pod/distribution/CGI/lib/CGI.pod' },
           'CPAN'),
        ' to get a new version.';
}

print hr;
showEnv;

print hr,
      end_html;

1;

