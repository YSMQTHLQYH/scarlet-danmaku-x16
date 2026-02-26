count_angle = 256;
count_speed = 8;
count_page = 2;

angle = 0:(2*pi)/count_angle:2*pi;
angle = angle(1:count_angle); #octave starts counting at 1 instead of 0

speed = [0.3:0.1:1;
        1.125:0.125:2];

for j = 1:count_page
  for i = 1:count_speed
    s = speed(j,i);
    #angle = 0 is down, increasing counterclockwise
    #+y is down, -y is up: x(0) = 0, y(0) = 1
    #+x ir right, -x is left: x(64) = 1, x(192) = -1, y(64)=y(192) = 0
    x_float = sin(angle);
    y_float = cos(angle);

    x_int = int16(x_float * i * 256);
    y_int = int16(y_float * i * 256);

    x_low(j, i, 1:count_angle) = uint8(bitand(x_int, 255));
    x_high(j, i, 1:count_angle) = int8(bitshift(x_int, -8));
    y_low(j, i, 1:count_angle) = uint8(bitand(y_int, 255));
    y_high(j, i, 1:count_angle) = int8(bitshift(y_int, -8));

  endfor
endfor

filename = "BULLET LOOKUP.BIN";
fid = fopen (filename, "wb");
# Do the actual I/O here...
for j = 1:count_page
  for i = 1:count_speed
    fwrite(fid, x_low(j, i, :), "uint8");
    fwrite(fid, x_high(j, i, :), "int8");
    fwrite(fid, y_low(j, i, :), "uint8");
    fwrite(fid, y_high(j, i, :), "int8");
  endfor
endfor
fclose (fid);
