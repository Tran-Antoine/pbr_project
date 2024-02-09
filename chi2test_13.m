obsFrequencies = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 831, 803, 773, 824, 784, 823, 805, 831, 771, 787, 804, 826, 1044, 7083, 28831, 27966, 6325, 1100, 790, 776; 2415, 2417, 2457, 2453, 2433, 2420, 2447, 2337, 2428, 2389, 2318, 2687, 4704, 13259, 24970, 24635, 12843, 4587, 2653, 2462; 4111, 4043, 4033, 3989, 3996, 4144, 4018, 4036, 4117, 4162, 4535, 5762, 9506, 16249, 22052, 21580, 16099, 9285, 5647, 4453; 6509, 5979, 5927, 5690, 5645, 5602, 5683, 5961, 5956, 6310, 7459, 9539, 12929, 16957, 19582, 19638, 16716, 12808, 9437, 7308; 10224, 9532, 8832, 8611, 8628, 8567, 8725, 8994, 9342, 10182, 11265, 12775, 14357, 15844, 16561, 16760, 15840, 14338, 12607, 11399 ];
expFrequencies = [ 24.5173, 12.9122, 8.22873, 6.22033, 5.4236, 5.43908, 6.27459, 8.35412, 13.2025, 25.3274, 58.9703, 188.093, 723.737, 2492.91, 5063.37, 4983.72, 2385.46, 682.932, 178.269, 56.5977; 9.21796e-05, 8.58176e-07, 1.40203e-08, 6.44334e-10, 1.2441e-10, 1.29041e-10, 7.14819e-10, 1.63956e-08, 1.03792e-06, 0.000112961, 0.0133331, 1.25197, 74.0528, 2006.43, 15832.5, 15098.5, 1775.81, 63.2193, 1.04005, 0.0109285; 2.39951e-28, 7.7915e-38, 0, 0, 0, 0, 0, 0, 1.91027e-37, 6.14686e-28, 1.74305e-18, 4.88764e-10, 0.00195387, 48.0674, 25452.1, 21256, 34.6293, 0.00114156, 2.3086e-10, 7.15681e-19; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6.9856e-32, 4.08867e-13, 0.193661, 35449, 19716, 0.0901872, 8.88245e-14, 8.31564e-33, 0; 1.35974e-14, 7.52875e-20, 1.46715e-24, 3.35183e-28, 2.90084e-30, 3.23916e-30, 4.47965e-28, 2.22793e-24, 1.23638e-19, 2.2911e-14, 4.02886e-09, 0.000202181, 1.31187, 830.053, 31416.9, 29011, 667.236, 0.944611, 0.000132877, 2.45796e-09; 800, 800, 800, 800, 800, 800, 800, 800, 800, 800.001, 800.053, 805.81, 1116.9, 6905.58, 28691.2, 27803.9, 6331.03, 1072.58, 804.816, 800.043; 2401.49, 2400.12, 2400.01, 2400, 2400, 2400, 2400, 2400.01, 2400.13, 2401.66, 2423.39, 2687.4, 4820.91, 13298.2, 25049.5, 24703.6, 12783.4, 4636.49, 2659.95, 2420.93; 4089.36, 4021.76, 4006.56, 4002.75, 4001.75, 4001.77, 4002.83, 4006.85, 4023.02, 4095.13, 4421.32, 5716.15, 9502.93, 16266.4, 22102.3, 21953.3, 15956, 9273.12, 5623.17, 4395.81; 6416.99, 5981.27, 5800.04, 5725.64, 5698.71, 5699.24, 5727.59, 5804.91, 5993.04, 6445.04, 7483.61, 9593.37, 13020.9, 16991.4, 19687.4, 19623.1, 16834.9, 12854.4, 9476.56, 7421.44; 10172.8, 9447.02, 8992.02, 8731.96, 8615.38, 8617.82, 8739.8, 9006.88, 9471.67, 10210.9, 11298.9, 12742.2, 14380.3, 15853.6, 16725.8, 16705.8, 15800.5, 14311.1, 12675.3, 11245.2 ];
colormap(jet);
clf; subplot(2,1,1);
imagesc(obsFrequencies);
title('Observed frequencies');
axis equal;
subplot(2,1,2);
imagesc(expFrequencies);
axis equal;
title('Expected frequencies');
