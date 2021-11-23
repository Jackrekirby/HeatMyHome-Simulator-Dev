clear all;
i = 0;
for w = 190
    disp(w);
    d = readmatrix(sprintf("c_surfaces/%i.csv", w));
    s = size(d);
    if min(s) > 1
        [X,Y] = meshgrid(1:s(1), 1:s(2));
        surf(X', Y', d);
        alpha 0.5;
        title(sprintf('Surface: %i', w));
        xlabel("TES Vol Index");
        ylabel("Solar Size Index");
        zlabel("NPC");
        i = i + 1;
        %saveas(gcf,sprintf("c_images/%i.png", i));
        pause;
    end
end