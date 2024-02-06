obsFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10306, 16650, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 509, 22304, 24509, 649, 3, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 234, 4647, 22841, 24190, 5283, 258, 7, 0, 0, 0; 821, 755, 806, 810, 794, 855, 791, 785, 829, 1023, 2790, 10527, 21335, 22008, 10899, 3001, 1059, 830, 843, 758; 2322, 2379, 2415, 2352, 2454, 2362, 2425, 2461, 2713, 3891, 7373, 14094, 20342, 20364, 14310, 7713, 3903, 2834, 2499, 2350; 4058, 4099, 4211, 3947, 4080, 4137, 4352, 4618, 5580, 7577, 11553, 16015, 19365, 19607, 16022, 11580, 7855, 5602, 4632, 4403; 6315, 6233, 6100, 6082, 6125, 6431, 6796, 7718, 9036, 11172, 13945, 16640, 18575, 18479, 16849, 14042, 11461, 9228, 7639, 6924; 10379, 9848, 9725, 9875, 10084, 10306, 10873, 11741, 12704, 13941, 15279, 16182, 16861, 16604, 16197, 15205, 13946, 12859, 11616, 10924 ];
expFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.87489e-36, 1.46396e-15, 0.0240044, 10291.6, 16425.4, 0.0577318, 8.4044e-15, 2.13668e-35, 0, 0, 0; 3.48208e-24, 7.76325e-28, 5.97717e-30, 5.33709e-30, 5.76053e-28, 2.26507e-24, 1.01829e-19, 1.59286e-14, 2.5025e-09, 0.000119621, 0.765301, 508.566, 22419, 24416.7, 637.304, 1.07312, 0.000184321, 4.1632e-09, 2.726e-14, 1.69701e-19; 4.85408e-08, 2.08967e-09, 3.75097e-10, 3.61053e-10, 1.87484e-09, 4.12044e-08, 2.59499e-06, 0.000300804, 0.0415556, 4.45549, 241.167, 4740.71, 23142.3, 23943.1, 5250.85, 281.6, 5.40668, 0.0515231, 0.000373694, 3.17032e-06; 800.012, 800.002, 800.001, 800.001, 800.002, 800.011, 800.097, 801.253, 818.257, 1032.85, 2834.07, 10331.7, 21466.9, 21806.9, 10828.5, 3010.09, 1058.77, 820.522, 801.41, 800.108; 2404.42, 2401.71, 2401.03, 2401.02, 2401.66, 2404.2, 2415.14, 2468.1, 2729.4, 3855.38, 7396.02, 14132.3, 20320.3, 20481.6, 14463.1, 7633.28, 3946.78, 2752.83, 2472.98, 2416.12; 4115.77, 4065.71, 4048.59, 4048.27, 4064.45, 4112.36, 4241.69, 4592.83, 5508.54, 7599.99, 11344.7, 16008, 19362.3, 19443.4, 16202.5, 11543.5, 7729.68, 5570.87, 4617.76, 4250.93; 6427.13, 6201.89, 6108.14, 6106.25, 6195.43, 6413.37, 6850.05, 7666.72, 9083.93, 11262, 14041.2, 16753.4, 18458.4, 18497.9, 16856.6, 14169.1, 11375, 9163.6, 7714.94, 6876.61; 10335.2, 9986.39, 9819.42, 9815.89, 9975.38, 10315.6, 10868, 11665.4, 12713.1, 13951.7, 15222.5, 16287, 16902.4, 16916.3, 16325.2, 15275.7, 14008.6, 12764.5, 11706.5, 10897.8 ];
colormap(jet);
clf; subplot(2,1,1);
imagesc(obsFrequencies);
title('Observed frequencies');
axis equal;
subplot(2,1,2);
imagesc(expFrequencies);
axis equal;
title('Expected frequencies');