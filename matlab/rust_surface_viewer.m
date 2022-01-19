% SURFACE VIEWER AND SAVER - FULLY WORKING
clear all; clc;
cd
dir = "../rust_simulator/tests/surfaces";

i = 0;
figure;
for w = 151:1000
    disp(w);
    d = readmatrix(sprintf("%s/%i.csv", dir, w));
    s = size(d);
    if min(s) > 1
        f = readmatrix(sprintf("%s/o%i.csv", dir, w));
        [X,Y] = meshgrid(1:s(1), 1:s(2));
        surf(X', Y', d);
        hold on;
        plot3(X, Y, f, 'rs');
        hold off;
        alpha 0.5;
        title(sprintf('Surface: %i', w));
        ylabel("TES Vol Index");
        xlabel("Solar Size Index");
        zlabel("NPC");
        i = i + 1;
        %saveas(gcf,sprintf("c_images/%i.png", i));
        pause;
    end
end