fid=fopen('./Big5-ZhuYin.map', 'r', 'n', 'big5');
totalNum = 13009;
tline=fgetl(fid);
tline=native2unicode(tline);
c = 1;
texts{c,1}= tline;
map = containers.Map;

map(tline(3)) = [tline(1)];
delimpos =  strfind(tline, '/');
temp = tline(delimpos+1);
[i, j] = size(temp);
for m = 1:j
    if map.isKey(temp(m))
    else
        map(temp(m)) = [tline(1)];
    end
end

while c ~= totalNum
    c = c+1;
    tline=fgetl(fid);
    tline = native2unicode(tline);
    texts{c, 1} = tline;
    if map.isKey(tline(3))
        map(tline(3)) = [map(tline(3)), tline(1)];
    else
        map(tline(3)) = [tline(1)];
    end
    delimpos =  strfind(tline, '/');
    temp = tline(delimpos+1);
    [i, j] = size(temp);
    for m = 1:j
        if map.isKey(temp(m))
        else
            map(temp(m)) = [tline(1)];
        end
    end
end

fclose(fid);

fid=fopen('./ZhuYin-Big5.map', 'w', 'n', 'big5');

index = map.keys;
for m=1:35
    word = map(index{m});
    fprintf(fid, '%s ',index{m});
    [a, b] = size(word);
    for n=1:b
        fprintf(fid, ' %s', word(n));
    end
    fprintf(fid, '\n');
    for n=1:b
        fprintf(fid, '%s  %s\n', word(n), word(n));
    end
end

fclose(fid);


