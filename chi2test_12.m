obsFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 404, 6805, 66, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 3, 191, 4847, 12245, 2560, 61, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 14, 235, 2238, 9139, 13904, 6942, 1356, 99, 6, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 50, 270, 1528, 5357, 11408, 14421, 9864, 3990, 1008, 163, 27, 2, 1, 0, 0; 765, 873, 797, 849, 954, 1294, 2171, 4545, 8627, 12938, 14605, 12044, 7539, 3723, 1735, 1086, 901, 809, 773, 820; 2502, 2513, 2590, 2674, 3125, 3970, 5643, 8313, 11735, 14889, 15645, 14000, 11033, 7463, 5026, 3760, 3109, 2759, 2671, 2476; 4815, 4755, 5025, 5276, 6072, 7305, 9066, 11544, 14116, 15539, 16197, 15436, 13622, 10830, 8506, 6935, 5954, 5159, 4856, 4659; 7607, 7745, 8026, 8693, 9617, 10727, 12313, 13889, 15496, 16630, 16895, 16510, 15049, 13664, 11822, 10456, 9276, 8499, 7989, 7529; 11737, 12149, 12251, 12546, 13219, 14162, 14636, 15781, 16224, 16529, 16730, 16524, 16098, 15339, 14448, 13789, 13105, 12388, 12167, 11801 ];
expFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 5.25423e-32, 1.29532e-19, 2.45997e-09, 0.0350987, 400.417, 6656.88, 59.9312, 0.0011099, 1.03689e-11, 1.21668e-22, 2.21934e-35, 0, 0, 0, 0; 3.27128e-17, 1.7816e-16, 1.03689e-14, 3.48152e-12, 3.34533e-09, 4.40585e-06, 0.00405725, 1.66468, 198.604, 4762.68, 12454.6, 2503.76, 67.3596, 0.391052, 0.000749536, 7.03705e-07, 5.40458e-10, 6.8711e-13, 3.00878e-15, 9.00405e-17; 4.34896e-06, 8.70448e-06, 4.84157e-05, 0.000616087, 0.0137881, 0.4011, 10.9894, 210.43, 2177.22, 9298.45, 13874.9, 7058.5, 1286.4, 103.511, 4.80756, 0.167608, 0.00597829, 0.000300394, 2.8531e-05, 6.57018e-06; 0.0644283, 0.0950036, 0.249589, 1.05547, 6.20254, 42.5518, 282.951, 1508.64, 5401.49, 11512.9, 14121.1, 9992.52, 4077.08, 1013.02, 176.325, 25.8345, 3.85056, 0.701786, 0.185195, 0.0811504; 808.027, 810.192, 818.472, 844.955, 934.131, 1238.45, 2187.22, 4562.31, 8725.21, 13070.3, 14604.4, 12107.9, 7531.64, 3773.25, 1842.51, 1122.86, 899.948, 834.944, 815.37, 809.251; 2525.42, 2546, 2613.08, 2774.65, 3147.36, 3970.89, 5612.97, 8337.14, 11767.5, 14645.1, 15565.6, 14046, 10877.3, 7538.76, 5092.32, 3697.37, 3020.87, 2719.37, 2589.59, 2537.27; 4678.31, 4746.24, 4946.11, 5347.23, 6073.46, 7291.11, 9127.68, 11497.6, 13949.2, 15761.1, 16309.2, 15397.2, 13350.2, 10856.6, 8596.11, 6922.45, 5847.07, 5219.15, 4879.25, 4717.89; 7644.87, 7762.1, 8083.5, 8652.89, 9527.27, 10745.3, 12272.8, 13948, 15478.2, 16524.1, 16829.7, 16318.6, 15118.4, 13518.2, 11857.4, 10399.8, 9270.76, 8479.88, 7979.45, 7713.74; 11829.3, 11933.5, 12204.9, 12643.5, 13241.3, 13970.3, 14773.2, 15560.3, 16221.2, 16649.6, 16771.9, 16566.7, 16069.8, 15365.7, 14564.3, 13773, 13073.7, 12515.1, 12119.2, 11890.9 ];
colormap(jet);
clf; subplot(2,1,1);
imagesc(obsFrequencies);
title('Observed frequencies');
axis equal;
subplot(2,1,2);
imagesc(expFrequencies);
axis equal;
title('Expected frequencies');