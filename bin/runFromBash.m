% Comparing TOUCH algorithms

for epsilon = 50:50:200
    for objnum = 1000:1000:2000
        for alg = [4 6 7 8]
            args = strcat({'-a '},{int2str(alg)},{' -p 100 -e '},{int2str(epsilon)},{' -b 3 -t 1 -J 2 '},...
                {'-i ../data/RandomData-10K.bin ../data/RandomData-Normal-500-250-10K.bin -n '},...
                {int2str(objnum)},{' '},{int2str(objnum)});
            setenv('SJARGS',args{1,1});

            !./SpatialJoin $SJARGS
        end
    end
end

 

