
#This is a dodgy port of the perl module Geo::Coordinates::OSTN02 by Toby Thurston and the original license was "same as perl"
#The transform is Crown Copyright (C) 2002, although how an algorithm is copyrightable is beyond me
#The python port is (c) 2010 Tim Sheerman-Chase and released under GPL version 2 and GPL version 3.

import os.path

MAX_EASTING  =  700000
MAX_NORTHING = 1250000

MIN_X_SHIFT =  86.275
MIN_Y_SHIFT = -81.603
MIN_Z_SHIFT =  43.982

def ostn():
	datafile = "ostn02data.txt" 
	if not os.path.exists(datafile):
		datafile = "pyostn02/ostn02data.txt" 

	fi = open(datafile,"rt")
	lines = fi.readlines()
	out = {}
	for line in lines:
		line = line.rstrip('\r\n')
		#print line
		ne = line[:6]
		offset = (int(line[6:10],16),int(line[10:14],16),int(line[14:18],16))
		out[ne] = offset
	return out

ostn_data = ostn() # load all the data from below
ostn_shift_for= {}

'''
sub ETRS89_to_OSGB36 {
    return unless defined wantarray;

    my ($x,  $y,  $z ) = (@_, 0);

    if ( 0 <= $x && $x <= MAX_EASTING && 0 <= $y && $y <= MAX_NORTHING ) {
        my ($dx, $dy, $dz) = _find_OSTN02_shifts_at($x,$y);
        ($x, $y, $z) = _round_to_nearest_mm($x+$dx, $y+$dy, $z-$dz); # note $z sign differs
    }
    else {
        #carp "[OSTN02 is not defined at ($x, $y), coordinates unchanged]";
    }

    return ($x, $y, $z) if wantarray;
    return "$x $y $z";
}
'''

def OSGB36_to_ETRS89 (x0, y0, z0 = 0.0):
    epsilon = 0.00001
    shifts = _find_OSTN02_shifts_at(x0,y0)
    if shifts == None: return None
    (dx, dy, dz) = shifts
    (x,  y,  z ) = (x0-dx, y0-dy, z0+dz)
    (last_dx, last_dy) = (dx, dy)
    #APPROX:
    while 1:
        (dx, dy, dz) = _find_OSTN02_shifts_at(x,y)
        (x, y) = (x0-dx, y0-dy)
        if abs(dx-last_dx)<epsilon and abs(dy-last_dy)<epsilon: #last APPROX 
		break	
        (last_dx, last_dy) = (dx, dy)

    (x, y, z) = _round_to_nearest_mm(x0-dx, y0-dy, z0+dz);
    return (x, y, z)


def _round_to_nearest_mm(x,  y,  z):

    x = int(x*1000+0.5)/1000
    y = int(y*1000+0.5)/1000
    z = int(z*1000+0.5)/1000
    return (x, y, z)


def _find_OSTN02_shifts_at(x,y):

    e_index = int(x/1000)
    n_index = int(y/1000)

    s0_ref = _get_ostn_ref(e_index+0, n_index+0)
    s1_ref = _get_ostn_ref(e_index+1, n_index+0)
    s2_ref = _get_ostn_ref(e_index+0, n_index+1)
    s3_ref = _get_ostn_ref(e_index+1, n_index+1)
    if s0_ref == None or s1_ref == None or s2_ref == None or s3_ref == None: return None

    x0 = e_index * 1000
    y0 = n_index * 1000

    dx = x - x0 # offset within square
    dy = y - y0

    t = dx/1000
    u = dy/1000

    f0 = (1-t)*(1-u)
    f1 =    t *(1-u)
    f2 = (1-t)*   u
    f3 =    t *   u

    se = f0*s0_ref[0] + f1*s1_ref[0] + f2*s2_ref[0] + f3*s3_ref[0]
    sn = f0*s0_ref[1] + f1*s1_ref[1] + f2*s2_ref[1] + f3*s3_ref[1]
    sg = f0*s0_ref[2] + f1*s1_ref[2] + f2*s2_ref[2] + f3*s3_ref[2]

    if se*sn*sg==0:
        print "[OSTN02 defined as zeros at ($x, $y), coordinates unchanged]"
    #print se, sn, sg
    return (se, sn, sg)


def _get_ostn_ref(x,y):

    key = "%03x%03x" % (y, x)
    if key in ostn_shift_for:
	return ostn_shift_for[key]

    if key in ostn_data:
	data = ostn_data[key]
	data2 = (data[0]/1000.0 + MIN_X_SHIFT,data[1]/1000.0 +MIN_Y_SHIFT,data[2]/1000.0 + MIN_Z_SHIFT)
	ostn_shift_for[key] = data2
        return data2

'''
    my $lo = 0;
    my $hi = @ostn_data-1;
    my $mid;
    while ($lo<=$hi) {
        $mid = int(($hi+$lo)/2);    # warn "$key -> $lo $mid $hi\n";
        my $k = substr($ostn_data[$mid],0,6);

        my @data = map { hex($_)/1000 }
                       ( substr($ostn_data[$mid], 6,4),
                         substr($ostn_data[$mid],10,4),
                         substr($ostn_data[$mid],14,4) );

        $data[0] += MIN_X_SHIFT;
        $data[1] += MIN_Y_SHIFT;
        $data[2] += MIN_Z_SHIFT;

        $ostn_shift_for{$k} = \@data;

        if    ( $k lt $key ) { $lo = $mid+1 }
        elsif ( $k gt $key ) { $hi = $mid-1 }
        else {
            return $ostn_shift_for{$k};
        }
    }
    # since we leave out all the zeros in the data set to save space, we might run off the bottom
    # and so we need to return zeros here
    my @data = (0,0,0);
    return $ostn_shift_for{$key} = \@data;



'''


'''

1;

=pod

=head1 NAME

Geo::Coordinates::OSTN02 - An implementation of the OSGB's OSTN02 transformation

=head1 SYNOPSIS

  use Geo::Coordinates::OSTN02 qw/OSGB36_to_ETRS89 ETRS89_to_OSGB36/;

  ($x, $y, $z) = OSGB36_to_ETRS89($e, $n, $h);
  ($e, $n, $h) = ETRS89_to_OSGB36($x, $y, $z);

=head1 DESCRIPTION

The purpose and use of these modules is described briefly in the companion
L<Geo::Coordinates::OSGB> modules.  In essence they implement in Perl the
Ordnance Survey's OSTN02 transformation which is part of the current
definition of the British National Grid.  You are strongly recommended to
read the official public guides and other introductory documents that are
published by the Ordnance Survey of Great Britain with a wealth of other
technical information about the OSTN02 transformation.

=head1 FUNCTIONS

The following functions can be exported from the C<Geo::Coordinates::OSTN02>
module:

    OSGB36_to_ETRS89
    ETRS89_to_OSGB36

None is exported by default.

=over 4

=item OSGB36_to_ETRS89(easting, northing, [elevation])

Transforms from normal OSGB36 grid references to a pseudo-grid based on the WGS84/ETRS89 geoid
model, which can then be translated to lat/long coordinates using C<grid_to_ll()> with the 'WGS84'
parameter.

   my $elevation = '100'; # in metres
   my ($e, $n) = parse_grid_trad('TQ304293');
   my ($x, $y, $z) = OSGB36_to_ETRS89($e, $n, $elevation);
   my ($lat, $lon) = grid_to_ll($x, $y, 'ETRS89'); # or 'WGS84'

Elevation will default to 0 metres if you omit it.

=item ETRS89_to_OSGB36(x, y, [z])

Transforms WGS84/ETRS89 pseudo-grid coordinates into OSGB36 grid references.

  my ($lat, $lon, $height) = (51.5, -1, 10); # Somewhere in London
  my ($x, $y) = ll_to_grid($lat, $lon, 'ETRS89'); # or 'WGS84'
  my ($e, $n, $elevation) = ETRS89_to_OSGB36($x, $y, $height);

=back

=head1 IMPLEMENTATION NOTES

Since 2002 the British Ordnance Survey have defined the UK national grid not
as a projection from their own model of the earth (the Airy 1830 geoid,
revised 1936, known as OSGB36), but as a simple table of calculated
differences from a projection based on the European standard geoid ETRS89
(which is for Europe a functional equivalent of the international WGS84
geoid).  This revision is known as OSGM02 and the transformation is called OSTN02.

The idea is that you project your WGS84 latitude and longitude
coordinates into WGS84 (or ETRS89) pseudo-grid references, and then look up the
appropriate three dimensional correction in the OS table, and add the results
to your grid reference to give you a standard British National Grid reference
that you can use with British OS maps.  Going back the other way is slightly
more complicated, as you have to work backwards, but a simple iteration will
do the job.  This package implements these lookups and adjustments.  You give
it a three dimensional grid reference (easting, northing, and altitude, all
in metres) and the package returns it corrected from one system to the other.

The problem in the implementation is that the table is huge, and most of it
is empty as the OS have chosen to leave the transformation undefined for
areas that are off shore.  So this package only works properly for grid
references that are actually on one or more OS maps.  The complete table
(including all the 0 lines) contains nearly 1 million lines with six data
points and a key.  In text form as supplied by the OS that is about 36M bytes
of table.  By leaving out the lines where the transformation is undefined, omitting
a couple of redundant fields, and storing everything as hex strings, this module
brings the amount of data down to just over 6M bytes, which loads in about 1 second
on my test system.  It would be possible to compress the data down to 3M bytes
by storing it as packed decimals, but then it would be difficult to include
inline in this module, as it would break every time I edited it.

The data is stored below, after the __DATA__ line.  Each line is 18 bytes long and represents the
transformation parameters for an individual grid square of 1km by 1km.  Each line contains
five fields all expressed as hexadecimal integers.

    Start   Length  Meaning
    0       3       The northing of the square in km
    3       3       The easting of the square in km
    6       4       The x-offset in mm (easting)
    10      4       The y-offset in mm (northing)
    14      4       The z-offset in mm (elevation)

To keep the numbers small and positive the values given for the offsets are
actually the amount that they exceed the respective smallest values in the data set.
Currently these minima are x:  86275mm, y:  -81603mm, and z:  43982mm.  So
when we read a line from the data we have to add these minima to the values,
convert to decimal, and multiply by 1000 to get back to metres.

When you load the OSTN02 module, the first thing it does is to load all 309,798 lines
into an array called @ostn_data by simply doing this.

    our @ostn_data = <DATA>;

This is why it takes over a second to load the module, but once loaded it's all very fast.

When we need to find the data values for a given grid reference, we work out the appropriate
grid square by truncating the easting and northing to the next lowest whole kilometer,
and pass these as the argument to the C<_get_ostn_ref> subroutine.  This is the only subroutine that
actually touches the data.

The core of the subroutine is a binary search.  We work out the key by converting the
northing and easting to hexadecimal and concatenating them.  We add leading zeros so that each value
is exactly three bytes long and the combined key is exactly six bytes long.  The maximum value for
easting in the OSGB grid is 700km and for northing 1250km, which are 2BC and 4E2, so each fits
into three bytes. (And by the time this is called we have already checked that the values don't
exceed the maxima).

This works pretty quickly, the only slow bit is loading the array at the
beginning, but it is much faster and needs *much* less memory than loading all
the data into the hash. (This would be simpler, and is what the original
version did, but it was too slow to be usable and meant that the tests failed
on many smaller systems as part of CPAN testing).  We do still use a hash,
but only to cache lines that we've already found.  Read the code for details.
This only gives a tiny speed up in general, so I might remove it in future versions.

Loading the array takes about 1.2 seconds on my Windows machine (a 2.8G Hz
Intel Pentium M processor with 2G byte of memory) and just under 0.5 seconds
on my Linux system (a 2.8G Hz Intel Pentium 4 processor with 512M bytes of
memory).  I think this probably says more about speed of the disks I have
(and probably the efficiency of Perl under Linux), but your results should be
comparable.  Once the data is loaded, calling the routines is reasonably
quick.

=head1 BUGS

Please report any to toby@cpan.org

=head1 EXAMPLES

See the test routines included in the distribution.

=head1 AUTHOR

Toby Thurston ---  6 Nov 2008

toby@cpan.org

=head1 SEE ALSO

See L<Geo::Coordinates::OSGB> for the main routines.
=cut
'''

