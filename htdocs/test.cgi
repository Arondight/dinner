#!/usr/bin/env perl

package main;

use warnings;
use strict;
use 5.010;

use CGI qw/:standard :html3/;

my @rows;

print header ('text/html'),
      start_html (-title => "CGI Tests");

print hr;
@rows = th ['Key', 'Value'];
push @rows, td [$_, param $_]
  for sort &param;
print table {-border=>undef, -width=>'25%'},
            caption b 'Parameters for CGI are:',
            Tr \@rows;

print hr;
@rows = th ['Variables', 'Value'];
push @rows, td [$_, $ENV{$_}]
  for sort keys %ENV;
print table {-border=>undef, -width=>'25%'},
            caption b 'Environment for CGI are:',
            Tr \@rows;


print end_html;

1;

