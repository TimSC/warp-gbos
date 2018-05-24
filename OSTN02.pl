#! /usr/bin/perl -w

use strict;
use warnings;

use lib 'lib';
use Geo::Coordinates::OSGB qw/
        parse_ISO_ll
        ll_to_grid
        shift_ll_from_WGS84
        format_ll_trad
        format_grid_trad
        format_grid_landranger
        grid_to_ll
        format_ll_ISO/;

use Geo::Coordinates::OSTN02 qw/OSGB36_to_ETRS89 ETRS89_to_OSGB36/;

sub OSTN02latlontogrid
{
my ($lat, $lon, $height) = ($_[0], $_[1], 0);
my ($x, $y) = ll_to_grid($lat, $lon, 'WGS84');
my ($e, $n, $elevation) = ETRS89_to_OSGB36($x, $y, $height);

return ($e, $n);
}

sub OSTN02latlontogridstr
{
	my ($e, $n) = OSTN02latlontogrid($_[0], $_[1]);
	my $out = sprintf("%f %f", $e, $n);
	return ($out);
}

sub OSTN02gridtolatlon
{
	my ($x, $y, $z) = (0,0,0);
	eval{($x, $y, $z) = OSGB36_to_ETRS89($_[0], $_[1], 0.0);};
	#warn $@ if $@;
	my ($gla, $glo) = grid_to_ll($x, $y, 'WGS84');
	return ($gla, $glo);
}

sub OSTN02gridtolatlonstr
{
	my ($gla, $glo) = OSTN02gridtolatlon($_[0], $_[1]);
	my $out = sprintf("%f %f", $gla, $glo);
	return ($out);
}

if($#ARGV >= 1) 
{
	my ($lonin, $latin) = @ARGV;
	my ($e, $n) = OSTN02latlontogrid($lonin, $latin);
	printf "%d %d\n", $e, $n;
}

