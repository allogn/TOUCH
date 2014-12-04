#!/usr/bin/perl
use strict; use warnings;
print "Content-type:text/html\n\n";

my $a = new CGI;

use CGI qw(:standard);
my $mineps = param('minEpsilon') || 15;
my $deps = param('deltaEpsilon') || 5;
my $maxeps = param('maxEpsilon') || 25;
my $minnum = param('minNum') || 1000;
my $dnum = param('deltaNum') || 1000;
my $maxnum = param('maxNum') || 2000;

my $data1 = 'RandomData-10K.bin';
my $data2 = 'RandomData-Normal-500-250-10K.bin';
system('rm ./SJ.csv');
my $result = '';
for (my $epsilon = $mineps; $epsilon <= $maxeps; $epsilon = $epsilon + $deps)
{
    for (my $objnum = $minnum; $objnum <= $maxnum; $objnum = $objnum + $dnum)
    {
        foreach my $alg (4,6,8)
	{
            $result = `/srv/www/touch/SpatialJoin -a $alg -p 100 -e $epsilon -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum`;
        }
    }
}
system('rm ./timings.png');
system('rm ./resultnum.png');
system('python ./generate.py');

my $filename = 'out.html';
if (open(my $fh, '<:encoding(UTF-8)', $filename)) {
  while (my $row = <$fh>) {
    chomp $row;
    print "$row\n";
  }
} else {
  warn "Could not open file '$filename' $!";
}
