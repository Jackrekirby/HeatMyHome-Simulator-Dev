clear all;
clc;

filename = "../rust_simulator/tests/performance.csv";
t = readtable(filename);
%%
clc
make_fig()
plot(t.Nodes, t.Opt_Elapsed/1000, 'g.');

hold on;
plot(t.Nodes, t.No_Opt_Elapsed/1000, 'r.');

x1 = linspace(0, 3e4);
p = polyfit(t.Nodes, t.No_Opt_Elapsed/1000, 1)
y1 = polyval(p,x1);
plot(x1, y1, 'b-');

p = polyfit(t.Nodes, t.Opt_Elapsed/1000, 2)
y1 = polyval(p,x1);
plot(x1, y1, 'b-');

hold off;
grid on;
grid minor;
axis square;
xlabel('No. Nodes');
ylabel('Run Time (s)');
title({'Surface Optimisation Performance', 'Comparison with No. Nodes'});

%%
clc
make_fig()
plot(t.Nodes, t.Gain, 'g.');
hold on;
x1 = linspace(0, 3e4);
p = polyfit(t.Nodes, t.Gain, 2)
y1 = polyval(p,x1);
plot(x1, y1, 'b-');
hold off;

hold off;
grid on;
grid minor;
axis square;
xlabel('No. Nodes');
ylabel('Gain');
title('Surface Optimisation Performance');


function make_fig()
    ss = get(0,'ScreenSize');
    ssr = 600;
    figure('Position', [ss(3:4)/2 - ssr/2, ssr, ssr]);
end
