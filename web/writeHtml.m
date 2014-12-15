html = runFromBash('RandomData-10K.bin', 'RandomData-Normal-500-250-10K.bin');

fid = fopen('out.html', 'w');
fprintf(fid, '%s', html);
fclose(fid);
