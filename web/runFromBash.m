function html=runFromBash(filename_in1, filename_in2)% Comparing TOUCH algorithms

!rm ./SJ.csv
for epsilon = 5:50:105
    for objnum = 100:1000:2100
        for alg = [4 6 8]
            args = strcat({'-a '},{int2str(alg)},{' -p 100 -e '},{int2str(epsilon)},{' -b 3 -t 1 -J 2 '},...
                {'-i '},{filename_in1},{' '},{filename_in2},{' -n '},...
                {int2str(objnum)},{' '},{int2str(objnum)});
            setenv('SJARGS',args{1,1});

            !./SpatialJoin $SJARGS
        end
    end
end

log_file = 'SJ.csv';
T = readtable(log_file,'Delimiter',',');

alg = T{:,'Algorithm'};
epsilon = T{:,'Epsilon'};
objn = T{:,'x_A'};
times = T{:,'tComparing'};
results = T{:,'Results'};

sample_epsilon = epsilon(1);

% first pic : 4 plots of alg_time(objn) (epsilon fixed)
% sec pic : 1 plot of results_num(epsilon) (alg and objnum fixed)

objnum = objn(epsilon == sample_epsilon & strcmp(alg,'TOUCH'));
touchtime = times(epsilon == sample_epsilon & strcmp(alg,'TOUCH'));
ctouchtime = times(epsilon == sample_epsilon & strcmp(alg,'cTOUCH'));
retouchtime = times(epsilon == sample_epsilon & strcmp(alg,'reTOUCH'));

objnummax = max(objnum);
epsilons = epsilon(objn == objnummax & strcmp(alg,'TOUCH'));
resultsnum = results(objn == objnummax & strcmp(alg,'TOUCH'));

frc_out= 'timings.png';
mae_out = 'resultnum.png';

h = figure;
set(h,'Visible','off');
hold('on');
plot(objnum,touchtime,'r-', 'Linewidth', 2); 
plot(objnum,ctouchtime,'g-', 'Linewidth', 2); 
plot(objnum,retouchtime,'b-', 'Linewidth', 2); 
axis('tight');
xlabel('Spatial object number, $\xi$', 'FontSize', 24, 'FontName', 'Times', 'Interpreter','latex');
ylabel('Time [s], $y$', 'FontSize', 24, 'FontName', 'Times', 'Interpreter','latex');
set(gca, 'FontSize', 24, 'FontName', 'Times');
legend('TOUCH', 'cTOUCH', 'reTOUCH');
saveas(h, frc_out, 'png');
close(h);

h = figure;
set(h,'Visible','off');
hold('on');
plot(epsilons,resultsnum,'b-', 'Linewidth', 2); 
axis('tight');
xlabel('$\epsilon$', 'FontSize', 20, 'FontName', 'Times', 'Interpreter','latex');
ylabel('Number of intersections', 'FontSize', 20, 'FontName', 'Times', 'Interpreter','latex');
set(gca, 'FontSize', 20, 'FontName', 'Times')
legend('Number of results');
saveas(h, mae_out, 'png');
close(h);


html=['<ul><li>The figure shows dependacy of joining time of TOUCH-like algorithms on numbers of objecs in sets. ', ...
    '<br>', ...
    '<img src="' frc_out '"  WIDTH="700" alt="difftouch"/> </li></ul>', ...
    '<ul><li>The figure shows the dependancy of number of objects that are closer than epsilon on epsilon.<br>', ...
    '<img src="' mae_out '"  WIDTH="700" alt="epsilondepend"/> </li></ul>'];


 

