function [Az,El,Pos] = PointingAngles(Lat,Long,Alt)

% Gs location is known
% put in payload location from aprs.fi
% antenna must point east at 90deg az

latft = @(x) x*364320; % turns lat (x) into ft
longft = @(x,y) y*acos(x*pi/180)*364320; 
% turns longitude (y) into ft at given lat (x)

% ENTER GS COORDINATES!!!!!
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
GS = [0 latft(44.0475);0  longft(44.0475,-94.4144);0  1000]; % lat,long,alt of GS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

lap = Lat;  
lop = Long;
ap = Alt;
P = [0 latft(lap);0 longft(44.0475,lop);0 ap];

G2P = P-GS;
Pos = G2P(:,2);

% for Az...
xy_vect = [G2P(2,2); G2P(1,2)];

% for El...
vert_vec = [norm(xy_vect); G2P(3,2)];

Az = 90 - atand(xy_vect(2)/xy_vect(1));
El = 90 + atand(vert_vec(2)/vert_vec(1));