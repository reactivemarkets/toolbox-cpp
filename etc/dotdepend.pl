#!/usr/bin/perl -w
# The Reactive C++ Toolbox.
# Copyright (C) 2013-2019 Swirly Cloud Limited
# Copyright (C) 2020 Reactive Markets Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

use strict;

# Usage: dotdepend.pl <lib>

my $cxx = $ENV{CXX} || 'g++';
my $lib = shift;
$lib or die "Library name not specified";

my $makedepend = "$cxx -std=c++17 -MM -MG -I. toolbox/$lib/*.[ch]pp";
my $tred = 'tred';

sub ltrim {
  $_[0] =~ s/^\s+//;
}

sub rtrim {
  $_[0] =~ s/\s+$//;
}

sub trim {
  ltrim($_[0]);
  rtrim($_[0]);
}

# Remove file extension.
sub rmext {
  $_[0] =~ s/\.\w+$//;
}

# Extract component and library from path.
sub parsedep {
  my ($path) = @_;
  # Remove leading slash.
  $path =~ s|^/||;
  my @parts = split('/', $path);
  my $comp = pop(@parts);
  rmext($comp) if defined $comp;
  my $lib = pop(@parts);
  return ($comp, $lib);
}

my ($comp, %comps);
my $cont = 0;

foreach (`$makedepend`) {

  chomp;

  my $deps;
  if ($cont) {

    # The dependency list is being continued from the previous line.
    $deps = $_;

  } else {

    # The dependency list is separated from the object file by the colon character.
    ($comp, $deps) = split(':');

    # The output of makedepend may lead with a blank line.
    next unless $comp;

    trim($comp);
    rmext($comp);

    $comps{$comp} = []
      unless defined $comps{$comp};
  }

  trim($deps);
  $cont = 0;

  foreach (split(/\s+/, $deps)) {

    trim($_);

    # A backslash encountered at the end of a line signifies a line continuation.
    if ($_ eq '\\') {
      $cont = 1;
    } else {

      my ($dcomp, $dlib) = parsedep($_);

      # If component within our library.
      if ($dlib eq $lib) {

        # Component dependency.
        #
        # Do not add dependency circular dependency to self. This may occur as a result of
        # including the header file associated with a component's source file.
        push(@{$comps{$comp}}, $dcomp)
          unless $comp eq $dcomp;
      } else {

        # Library dependency.
        push(@{$comps{$comp}}, $dlib);
      }
    }
  }
}

# The dot-file is piped through tred in order to remove transitive dependencies.
open(PIPE, "|$tred")
  or die "Failed to open tred pipe: $!";

# Libraries are depicted as boxes.
print PIPE <<EOD;
digraph G {
  ratio="auto";
  size="11.7,8.3!";
  node [shape=box];
  rankdir=BT
EOD

# Delete benchmarks and unit-tests.
foreach (keys %comps) {
  delete $comps{$_} if /[.](bm|ut)$/;
}

# Components are depicted as ovals.
print PIPE "  $_ [shape=oval];\n"
  foreach keys %comps;

while (my ($comp, $deps) = each %comps) {
  print PIPE "  $comp -> $_;\n"
    foreach @{$deps};
}

print PIPE "}\n";
close(PIPE);
