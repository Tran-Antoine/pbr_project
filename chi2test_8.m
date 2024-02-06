obsFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19161, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 535, 50272, 53, 0, 0, 0, 0, 0, 0, 0, 0; 1160, 1199, 1180, 1201, 1194, 1191, 1139, 1153, 1274, 7483, 64245, 3279, 1230, 1210, 1132, 1197, 1167, 1174, 1203, 1182; 3557, 3693, 3581, 3636, 3600, 3604, 3649, 3492, 3729, 18621, 62071, 11704, 3608, 3637, 3655, 3600, 3588, 3539, 3554, 3582; 5994, 6072, 5997, 6024, 6068, 5940, 5991, 6038, 7051, 25134, 49476, 19474, 6570, 6126, 5973, 5927, 5970, 6085, 6023, 6144; 8272, 8331, 8355, 8463, 8407, 8505, 8373, 8611, 11138, 23127, 32349, 20169, 10303, 8624, 8200, 8335, 8383, 8391, 8342, 8415; 10930, 10823, 10782, 10820, 10754, 10845, 10940, 11505, 12787, 15649, 17058, 15043, 12530, 11065, 11060, 10936, 10894, 10816, 10834, 10842 ];
expFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00743609, 18990, 2.3182e-08, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 4.48153e-22, 2.73536e-06, 531.685, 50038.5, 47.2789, 1.61943e-08, 1.34441e-25, 0, 0, 0, 0, 0, 0; 1200, 1200, 1200, 1200, 1200, 1200, 1200, 1200, 1201.14, 7515.03, 63802.1, 3276.07, 1200.14, 1200, 1200, 1200, 1200, 1200, 1200, 1200; 3600, 3600, 3600, 3600, 3600, 3600, 3600, 3600.07, 3726.35, 18588.1, 62381.5, 11784.8, 3640.48, 3600.02, 3600, 3600, 3600, 3600, 3600, 3600; 6000, 6000, 6000, 6000, 6000, 6000, 6000.06, 6013.73, 7130.37, 24943.5, 49931.6, 19227.2, 6573.57, 6005.57, 6000.01, 6000, 6000, 5999.99, 6000, 6000; 8400, 8400, 8400, 8400, 8400.01, 8400.33, 8409.49, 8614.36, 11188.6, 23004.9, 32541.7, 20206.2, 10276.4, 8527.47, 8405.28, 8400.18, 8400.01, 8399.99, 8400, 8400; 10804.3, 10804.7, 10806.1, 10809.2, 10817.8, 10842.6, 10943.2, 11375.2, 12854.3, 15640.7, 17107.1, 15130.5, 12479.3, 11249.7, 10913.2, 10835.6, 10815.6, 10808.5, 10805.8, 10804.6 ];
colormap(jet);
clf; subplot(2,1,1);
imagesc(obsFrequencies);
title('Observed frequencies');
axis equal;
subplot(2,1,2);
imagesc(expFrequencies);
axis equal;
title('Expected frequencies');