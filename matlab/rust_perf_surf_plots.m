clear all;
clc;

filename = "../rust_simulator/tests/perf_surf.csv";
t = readtable(filename);

Gain = mean(t.Gain)
NodesSearched = mean(t.NodesSearched)

%%
plot(t.TotalNodes, t.NodesSearched, 'g.');

hold on;

x1 = linspace(0, max(t.TotalNodes));
p = polyfit(t.TotalNodes, t.NodesSearched, 1)
y1 = polyval(p,x1);
plot(x1, y1, 'b-');
plot(x1, x1, 'b-');

hold off;
grid on;
grid minor;
axis square;
xlabel('No. Nodes');
ylabel('Nodes Searched');
title({'Surface Optimisation Performance', 'Comparison with No. Nodes'})

%%
clc;
mean_time = zeros(max(t.XNodes), max(t.YNodes));
nodes_num = ones(max(t.XNodes), max(t.YNodes));
for i = 1:length(t.NodesSearched)
    mean_time(t.XNodes(i), t.YNodes(i)) = mean_time(t.XNodes(i), t.YNodes(i)) + t.NodesSearched(i);
    nodes_num(t.XNodes(i), t.YNodes(i)) = nodes_num(t.XNodes(i), t.YNodes(i)) + 1;
end

mean_time = mean_time ./ nodes_num;
%%

% [X, Y] = meshgrid(1:max(t.YNodes), 1:max(t.XNodes));
% surf(X, Y, mean_time);

plot3(t.XNodes, t.YNodes, t.NodesSearched, 'g.');
grid on;
grid minor;
axis square;
xlabel('Solar Nodes');
ylabel('TES Nodes');
zlabel('Run Time (s)');