% SURFACE VIEWER AND SAVER - FULLY WORKING
clear all; clc;
cd
dir = "../rust_simulator/tests/surfaces";

i = 0;
figure;
for w = 128
    disp(w);
    d = readmatrix(sprintf("%s/%i.csv", dir, w));
    s = size(d);
    if min(s) > 1
        f = readmatrix(sprintf("%s/o%i.csv", dir, w));
        [X,Y] = meshgrid(1:s(1), 1:s(2));
        X = X';
        Y = Y';
        surf(X, Y, d);
        alpha 0.1
        [val, ind] = min(d(:))
        hold on;
        
        plot3(X(ind), Y(ind), val, 'go', 'MarkerSize', 5, 'LineWidth', 3);
        plot3(X, Y, f, 'rs');
        
        hold off;
        title(sprintf('Surface: %i', w));
        ylabel("TES Vol Index");
        xlabel("Solar Size Index");
        zlabel("NPC");
        i = i + 1;
        %saveas(gcf,sprintf("c_images/%i.png", i));
        pause;
    end
end