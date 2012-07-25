function aio=readlanguages

% Matlab script to convert a Languagues csv file to a C++ header file.
% Used in the development of Antares.
%
% H. Haselgrove, Nov. 2011.

fid=fopen('languages_25_july_12.csv','r','n','UTF8');

x=fread(fid,inf,'char');fclose(fid);

x=[10;x;10];
r = x<32;

x=char(x');


crs=find(r);

nc = length(crs);

all_items = cell(100,6);
ind=0;
for j =1:nc-1
    
    y=x(crs(j)+1:crs(j+1)-1);
    y=strtrim(y);
    if length(y)==0; continue;end
    
    
    z=parse_line(y);
    
    %disp(z);
    
    nz=length(z);
    if nz>=3
        label = z{1};
        en=z{3};
        fi='';de='';sv='';
        if nz>3; fi=z{4};end
        if nz>4; de=z{5};end
        if nz>6; 
            sv=z{7};
        end
        
        if any(label=='_')
            
            %fprintf('%s\n      %s\n      %s\n      %s\n',label,en,fi,de);
            ind=ind+1;
            all_items{ind,1} = label;
            all_items{ind,2} = en;
            all_items{ind,3} = fi;
            all_items{ind,4} = de;
            all_items{ind,5} = sv;
            
        end
        
    end
    
end
all_items = all_items(1:ind,:);
if nargout>0
    aio=all_items;
end
ni=ind;
f=fopen('c:\antares\language.h','w');
fprintf(f,'#pragma once\n');
fprintf(f,'using namespace System;\n');
fprintf(f,'\nnamespace Antares {\n');
fprintf(f,'public ref class lang\n{\npublic:\n');
ml=0;
for j=1:ni
    x = all_items{j,1};
    fprintf(f,'static String^ %s;\n',x);
    if length(x)>ml; ml=length(x);end
end


fprintf(f,'static void set_en_au(void){\n');
for j=1:ni
    x=all_items{j,1};
    sp=repmat(' ',ml-length(x),1);
    fprintf(f,'%s%s= "%s";\n',all_items{j,1},sp,all_items{j,2});
end
fprintf(f,'}\n');


fprintf(f,'static void set_fi(void){\n');
for j=1:ni
    x=all_items{j,1};
    y=all_items{j,3}; if length(y)==0; y=all_items{j,2};end
    sp=repmat(' ',ml-length(x),1);
    fprintf(f,'%s%s= "%s";\n',all_items{j,1},sp,y);
end
fprintf(f,'}\n');


fprintf(f,'static void set_de(void){\n');
for j=1:ni
    x=all_items{j,1};
     y=all_items{j,4}; if length(y)==0; y=all_items{j,2};end
    sp=repmat(' ',ml-length(x),1);
    if strcmp(all_items{j,1},'cb_move') && strcmp(y,'verschieben')
        yold=y;
        y='ver-\r\nschieben';
        fprintf('Replaced %s with %s  in German.\n',yold,y);
    end
    fprintf(f,'%s%s= "%s";\n',all_items{j,1},sp,y);
end
fprintf(f,'}\n');

fprintf(f,'static void set_sv(void){\n');
for j=1:ni
    x=all_items{j,1};
    y=all_items{j,5}; if length(y)==0; y=all_items{j,2};end
    sp=repmat(' ',ml-length(x),1);
    fprintf(f,'%s%s= "%s";\n',all_items{j,1},sp,y);
end
fprintf(f,'}\n');


fprintf(f,'static void set_en_gb(void){\n');
fprintf(f,'set_en_au();\n');
for j=1:ni
    x=all_items{j,1};
    
    y=all_items{j,2};
    z=strrep(y,'Program ','Programme ');
    z=strrep(z,'program ','programme ');
    
    %y=all_items{j,4}; if length(y)==0; y=all_items{j,2};end
    if ~strcmp(y,z)
        sp=repmat(' ',ml-length(x),1);
        fprintf(f,'%s%s= "%s";\n',all_items{j,1},sp,z);
        fprintf('Replaced %s with %s in English(UK)\n',y,z);
    end
end
fprintf(f,'}\n');





fprintf(f,'};\n}\n');

fclose(f);




function items=parse_line(y)

y=[y,' '];
items={};

ind=0;

ny=length(y);
inquote=0;
x='';
j=0;
while j<ny
    j=j+1;
    
    if ~inquote && y(j)==','
        ind=ind+1;
        items{ind}=deblank(x);
        x='';
        continue;
    end
    if y(j)~='"'
        x=[x,y(j)];
        continue;
    end
    
    nq = find(y(j:end)~='"',1,'first') - 1;
    
    switch nq
        case 0
            error('0')
        case 1
            inquote = 1-inquote;
        case 2
            if inquote; x=[x,'"']; end;
            j=j+1;
        case 3
            inquote=1-inquote;
            x=[x,'"'];
            j=j+2;
    end
    
end
x=deblank(x);
if length(x)>0
    items{ind+1}=x;
end
