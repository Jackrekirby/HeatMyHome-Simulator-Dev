clear all;
clc;

filename = "../rust_simulator/tests/performance.csv";
t = readtable(filename);

filename = "../rust_simulator/tests/input_list.csv";
t2 = readtable(filename);
tes_maxes = t2.Var8;

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

p1 = p

p = polyfit(t.Nodes, t.Opt_Elapsed/1000, 1)
y1 = polyval(p,x1);
plot(x1, y1, 'b-');

p2 = p

nodes_per_sec = 135 / 615

1e-3 / p1(1)
1e-3 / p2(1)
p1(1) / p2(1)

%plot(x1, x1 * nodes_per_sec, 'b-');
xlim([0, 3e4]);
ylim([0, 4])
hold off;
grid on;
grid minor;
axis square;
xlabel('Nodes');
ylabel('Runtime (s)');
title('Global Optimisation Performance');
legend(["Enabled", "Disabled", "Linear Regression"], 'location', 'northwest');
set(gca, 'FontName', 'FixedWidth', 'FontWeight', 'Bold');

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
    ssr = 400;
    figure('Position', [ss(3:4)/2 - ssr/2, ssr, ssr]);
end
