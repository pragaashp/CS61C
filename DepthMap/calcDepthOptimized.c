// CS 61C Fall 2014 Project 3

// include SSE intrinsics and OpenMP
#if defined(_MSC_VER)
#include <intrin.h>
#include <pthread.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "utils.h"

inline float displacement(int dx, int dy)
{
	return dx * dx + dy * dy;
}

void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{

	memset(depth, 0, imageWidth * imageHeight * sizeof(float)); // zero out depth map values
	int middle = (2*featureWidth + 1)/4 * 4 - featureWidth;
	int searchSpaceHeight = maximumDisplacement + featureHeight;
	int searchSpaceWidth = maximumDisplacement + featureWidth;
	int maxHeight = imageHeight - featureHeight; 
	int maxWidth = imageWidth - featureWidth;
	FILE *f = fopen("./sd.txt","wb"); 

	#pragma omp parallel
	{
		if (maximumDisplacement != 0) {
		
			#pragma omp for
			for (int y = featureHeight; y < maxHeight; ++y)
			{
				// upper and lower bounds of dy
				int lb_dy = (y < searchSpaceHeight) ? featureHeight - y : -maximumDisplacement;
				int ub_dy = (y + searchSpaceHeight >= imageHeight) ? maxHeight - y - 1: maximumDisplacement;
				
				for (int x = featureWidth; x < maxWidth; ++x) 
				{
					float minimumSquaredDifference = -1;
					int minimumDy = 0;
					int minimumDx = 0;
		
					// upper and lower bounds of dx
					int lb_dx = (x < searchSpaceWidth) ? featureWidth - x : -maximumDisplacement;
					int ub_dx = (x + searchSpaceWidth >= imageWidth) ? maxWidth - x - 1: maximumDisplacement;
					
					for (int dy = lb_dy; dy <= ub_dy; ++dy)
					{
						float firstCol = 0, squaredDifference = 0;

						for (int dx = lb_dx; dx <= ub_dx; ++dx)
						{
							if (dx == lb_dx) 
							{
								__m128 sd_v = _mm_setzero_ps(); // vectorized squaredDifference

								for (int boxX = -featureWidth; boxX < middle; boxX += 4)	
								{
									for (int boxY = -featureHeight; boxY <= featureHeight; ++boxY)
									{
										int leftX = x + boxX;
										int leftY = y + boxY;
										int rightX = leftX + dx;
										int rightY = leftY + dy;
										
										int L = leftY * imageWidth + leftX;
										int R = rightY * imageWidth + rightX;
										
										__m128 diff = _mm_sub_ps(_mm_loadu_ps(left + L), _mm_loadu_ps(right + R));
										sd_v = _mm_add_ps(sd_v, _mm_mul_ps(diff, diff));
									}

									if (boxX == -featureWidth)
									{
										firstCol = _mm_cvtss_f32(sd_v);
									}
								}

								float sd[4] = {0,0,0,0};
								_mm_storeu_ps(sd, sd_v);
								squaredDifference += (sd[0] + sd[1] + sd[2] + sd[3]);
							
								if ((minimumSquaredDifference >= squaredDifference) || (minimumSquaredDifference == -1)) {
									
									for (int boxY = -featureHeight; boxY <= featureHeight; ++boxY)
									{
										int leftX = x + middle;
										int rightX = leftX + dx;
										int leftY = y + boxY;
										int rightY = leftY + dy;
											
										int L = leftY * imageWidth + leftX;
										int R = rightY * imageWidth + rightX;
			
										float difference1 = left[L] - right[R];
										float sqdiff1 = difference1 * difference1;
			
										if (featureWidth & 1)
										{
											float difference2 = left[L + 1] - right[R + 1];
											float sqdiff2 = difference2 * difference2;

											float difference3 = left[L + 2] - right[R + 2];
											float sqdiff3 = difference3 * difference3;

											squaredDifference += sqdiff3 + sqdiff2 + sqdiff1;
			
										} else {
			
											squaredDifference += sqdiff1;
			
										}
									}
			
								}
							} else {

								squaredDifference -= firstCol;
								firstCol = 0;

								for (int boxY = -featureHeight; boxY <= featureHeight; ++boxY)
								{
									int leftX = x - featureWidth;
									int rightX = leftX + dx;
									int leftY = y + boxY;
									int rightY = leftY + dy;
										
									int L = leftY * imageWidth + leftX;
									int R = rightY * imageWidth + rightX;
								
									float difference1 = left[L] - right[R];
									firstCol += difference1 * difference1;

									float difference2 = left[L + featureWidth + featureWidth] - right[R + featureWidth + featureWidth];
									squaredDifference += difference2 * difference2;

								}
							}

							if ((minimumSquaredDifference == -1) || (minimumSquaredDifference > squaredDifference) || ((minimumSquaredDifference == squaredDifference) && (displacement(dx, dy) < displacement(minimumDx, minimumDy))))
							{
								minimumSquaredDifference = squaredDifference;
								minimumDx = dx;
								minimumDy = dy;
							}
						}
					}
		
					if (minimumSquaredDifference != -1)
					{
						int z = y * imageWidth + x;
						depth[z] = displacement(minimumDx, minimumDy);
						if (z % imageWidth == 0)
						{
							fprintf(f, "\n");
						}
						fprintf(f, "%.1f    ", depth[z]);
					}
				}
			}
		
			int rangeMin = featureHeight * imageWidth + featureWidth;
			int rangeMax = maxHeight * imageWidth + maxWidth;
			__m128 vec = _mm_setzero_ps();
		
			#pragma omp for
			for (int i = rangeMin; i < rangeMax; i += 4)
			{
				vec = _mm_loadu_ps(depth + i);
				vec = _mm_sqrt_ps(vec);
				_mm_storeu_ps(depth+i,vec);
			}
		}
	}
	fclose(f);
}