/*
 * file: q5.cpp
 * brief: Determine the frame rate of each of the 3 transformations, select deadline
 * 		  and check if the transformations meet the deadline
 * author: Devansh Mittal, Amreeta Sengupta
 * date: 04/02/2019
 * references: Canny Edge Detection: http://mercury.pr.erau.edu/~siewerts/cs415/code/computer-vision/simple-canny-interactive/ 
 * 			   Hough Transform: http://mercury.pr.erau.edu/~siewerts/cs415/code/computer-vision/simple-hough-interactive/
 * 			   Hough Circle Transform: http://mercury.pr.erau.edu/~siewerts/cs415/code/computer-vision/simple-hough-eliptical-interactive/
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define HRES 640
#define VRES 480

#define NUM_THREADS (3) 
#define FRAME (51)
#define HOUGHCIRCLE_DEADLINE (46)
#define HOUGH_DEADLINE (51)
#define CANNY_DEADLINE (46)
//#define CHECK_DEADLINE
#define CAL_FRAME_RATE

sem_t cam[NUM_THREADS];

CvCapture* capture;

void *hough_circle(void *)
{
	while(1)
	{
		sem_wait(&(cam[2]));
		printf("\n\nHough Circle Transform");
		cvNamedWindow("Hough Eliptical Transform", CV_WINDOW_AUTOSIZE);
		IplImage* frame;
		int dev=0;
		Mat gray;
		vector<Vec3f> circles;
		struct timeval time;
		double cur_time, past_time;
		int frame_count = 0;
		double fps = 0, spf = 0;
		int negative_jitter = 0;
		int positive_jitter = 0;
		double fps_sum = 0;
		double avg_frame_rate;
		int j = 0;
		int missed_deadline = 0;
		double avg_time;
		int total_pos_jitter = 0;
		int total_neg_jitter = 0;
		double avg_pos_jitter, avg_neg_jitter;
		
		while(j<FRAME)
		{		
			frame=cvQueryFrame(capture);
			if(!frame) break;
			
			gettimeofday(&time, 0);
			
			cur_time = (double)((double)time.tv_sec +((double)time.tv_usec/1000000.0));

			frame_count++;
			
			if(frame_count > 1)
			{
				spf = (cur_time-past_time);
				#ifdef CAL_FRAME_RATE
				fps = 1/spf;
				printf("\n[%lu.%06lu]Frame Rate: %fHz | Time for one frame: %fms", time.tv_sec, time.tv_usec, fps, (spf*1000));
				
				/* Determine avg frame rate */
				fps_sum += fps;
				if(frame_count == FRAME)
				{
					avg_frame_rate = fps_sum/50.0f;
					avg_time = 1/avg_frame_rate;
					printf("\nAverage Frame rate: %fHz | Average time for one frame: %fms", avg_frame_rate, (avg_time*1000));
				}
				#endif
			}
			past_time = cur_time;
			
			Mat mat_frame(frame);
			cvtColor(mat_frame, gray, CV_BGR2GRAY);
			GaussianBlur(gray, gray, Size(9,9), 2, 2);
			
			HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);
			
			for( size_t i = 0; i < circles.size(); i++ )
			{
			  Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			  int radius = cvRound(circles[i][2]);
			  // circle center
			  circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
			  // circle outline
			  circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
			}
			
			cvShowImage("Hough Eliptical Transform", frame);
			
			#ifdef CHECK_DEADLINE
			if(frame_count>1)
			{
				if((int)(spf*1000) < HOUGHCIRCLE_DEADLINE)
				{
					positive_jitter = (HOUGHCIRCLE_DEADLINE - (int)(spf*1000));
					total_pos_jitter += positive_jitter;
					printf("\nJitter: %d", positive_jitter);
				}
				else
				{
					negative_jitter = ((int)(spf*1000) - HOUGHCIRCLE_DEADLINE);
					printf("\nMissed Deadline at frame: %d | Jitter: %d", frame_count, negative_jitter);
					total_neg_jitter += negative_jitter;
					missed_deadline++;
				}
			}
			#endif
			
			char c = cvWaitKey(10);
			if( c == 27 )
			{
				printf("got quit\n"); 
				break;
			}
			
			j++;        
		}
		
		#ifdef CHECK_DEADLINE
		avg_pos_jitter = (double)(total_pos_jitter)/(50.0f-(double)missed_deadline);
		avg_neg_jitter = (double)(total_neg_jitter)/((double)missed_deadline);
		
		printf("\n\nAnalysis of Hough Eliptical Transform for 50 frames");
		printf("\nMissed Deadlines: %d", missed_deadline);
		printf("\nDeadlines Passed: %d", (50-missed_deadline));  
		printf("\nAverage positive jitter: %f", avg_pos_jitter);
		printf("\nAverage negative jitter: %f", avg_neg_jitter);
		#endif
		
		cvDestroyWindow("Hough Eliptical Transform");
		sem_post(&(cam[0]));
	}
}

void *hough(void *)
{
	while(1)
	{
		sem_wait(&(cam[1]));
		printf("\n\nHough Transform");
		cvNamedWindow("Hough Transform", CV_WINDOW_AUTOSIZE);
		IplImage* frame;
		int dev=0;
		Mat gray, canny_frame;
		vector<Vec4i> lines;
		struct timeval time;
		double cur_time, past_time;
		int frame_count = 0;
		double fps = 0, spf = 0;
		int jitter = 0;
		double fps_sum = 0;
		double avg_frame_rate;
		int j = 0;
		int missed_deadline = 0;
		int negative_jitter = 0;
		int positive_jitter = 0;
		double avg_time;
		int total_pos_jitter = 0;
		int total_neg_jitter = 0;
		double avg_pos_jitter, avg_neg_jitter;
		
		 while(j<FRAME)
		{    
			frame=cvQueryFrame(capture);
			if(!frame) break;
			
			gettimeofday(&time, 0);
			cur_time = (double)((double)time.tv_sec +((double)time.tv_usec/1000000.0));

			frame_count++;
			
			if(frame_count > 1)
			{
				spf = (cur_time-past_time);
				#ifdef CAL_FRAME_RATE
				fps = 1/spf;
				printf("\n[%lu.%06lu]Frame Rate: %fHz | Time for one frame: %fms", time.tv_sec, time.tv_usec, fps, (spf*1000));
				
				/* Determine avg frame rate */
				fps_sum += fps;
				if(frame_count == FRAME)
				{
					avg_frame_rate = fps_sum/50.0f;
					avg_time = 1/avg_frame_rate;
					printf("\nAverage Frame rate: %fHz | Average time for one frame: %fms", avg_frame_rate, (avg_time*1000));
				}
				#endif
			}
			past_time = cur_time;

			Mat mat_frame(frame);
			Canny(mat_frame, canny_frame, 50, 200, 3);
			
			HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

			for( size_t i = 0; i < lines.size(); i++ )
			{
			  Vec4i l = lines[i];
			  line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
			}
			
			cvShowImage("Hough Transform", frame);

			#ifdef CHECK_DEADLINE
			if(frame_count>1)
			{
				if((int)(spf*1000) < HOUGH_DEADLINE)
				{
					positive_jitter = (HOUGH_DEADLINE - (int)(spf*1000));
					total_pos_jitter += positive_jitter;
					printf("\nJitter: %d", positive_jitter);
				}
				else
				{
					negative_jitter = ((int)(spf*1000) - HOUGH_DEADLINE);
					printf("\nMissed Deadline at frame: %d | Jitter: %d", frame_count, negative_jitter);
					total_neg_jitter += negative_jitter;
					missed_deadline++;
				}
			}
			#endif
			
			char c = cvWaitKey(10);
			if( c == 27 )
			{
				printf("got quit\n"); 
				break;
			}
			j++;
		}
		
		#ifdef CHECK_DEADLINE
		avg_pos_jitter = (double)(total_pos_jitter)/(50.0f-(double)missed_deadline);
		avg_neg_jitter = (double)(total_neg_jitter)/((double)missed_deadline);
		printf("\n\nAnalysis of Hough Transform for 50 frames");
		printf("\nMissed Deadlines: %d", missed_deadline);
		printf("\nDeadlines Passed: %d", (50-missed_deadline));  
		printf("\nAverage positive jitter: %f", avg_pos_jitter);
		printf("\nAverage negative jitter: %f", avg_neg_jitter);
		#endif
		
		cvDestroyWindow("Hough Transform");
		sem_post(&(cam[2]));
	}
}

void *canny(void *)
{
	while(1)
	{
		sem_wait(&(cam[0]));
		printf("\n\nCanny Edge Detection");
		cvNamedWindow("Canny Edge Detection", CV_WINDOW_AUTOSIZE);
		IplImage* frame;
		int dev=0;
		int lowThreshold=0;
		int ratio = 3;
		struct timeval time;
		double cur_time, past_time;
		int frame_count = 0;
		double fps = 0, spf = 0;
		Mat canny_frame, timg_gray, timg_grad;
		int jitter = 0;
		double fps_sum = 0;
		double avg_frame_rate;
		int i = 0;
		int missed_deadline = 0;
		int negative_jitter = 0;
		int positive_jitter = 0;
		double avg_time;
		int total_pos_jitter = 0;
		int total_neg_jitter = 0;
		double avg_pos_jitter, avg_neg_jitter;
		
		while(i<FRAME)
		{		
			createTrackbar( "Min Threshold:", "Canny Edge Detection", &lowThreshold, 100, NULL);
			
			frame=cvQueryFrame(capture);
			if(!frame) break;
			
			gettimeofday(&time, 0);
			cur_time = (double)((double)time.tv_sec +((double)time.tv_usec/1000000.0));
			
			frame_count++;
			
			if(frame_count > 1)
			{
				spf = (cur_time-past_time);
				#ifdef CAL_FRAME_RATE
				fps = 1/spf;
				printf("\n[%lu.%06lu]Frame Rate: %fHz | Time for one frame: %fms", time.tv_sec, time.tv_usec, fps, (spf*1000));
				
				/* Determine avg frame rate */
				fps_sum += fps;
				if(frame_count == FRAME)
				{
					avg_frame_rate = fps_sum/50.0f;
					avg_time = 1/avg_frame_rate;
					printf("\nAverage Frame rate: %fHz | Average time for one frame: %fms", avg_frame_rate, (avg_time*1000));
				}
				#endif
			}
			past_time = cur_time;
			
			Mat mat_frame(frame);

			cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

			/// Reduce noise with a kernel 3x3
			blur( timg_gray, canny_frame, Size(3,3) );

			/// Canny detector
			Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, 3);

			/// Using Canny's output as a mask, we display our result
			timg_grad = Scalar::all(0);

			mat_frame.copyTo( timg_grad, canny_frame);

			imshow("Canny Edge Detection", timg_grad);
			
			#ifdef CHECK_DEADLINE
			if(frame_count>1)
			{
				if((int)(spf*1000) < CANNY_DEADLINE)
				{
					positive_jitter = (CANNY_DEADLINE - (int)(spf*1000));
					total_pos_jitter += positive_jitter;
					printf("\nJitter: %d", positive_jitter);
				}
				else
				{
					negative_jitter = ((int)(spf*1000) - CANNY_DEADLINE);
					total_neg_jitter += negative_jitter;
					printf("\nMissed Deadline at frame: %d | Jitter: %d", frame_count, negative_jitter);
					missed_deadline++;
				}
			}
			#endif
			
			char c = cvWaitKey(10);
			if( c == 27 )
			{
				printf("got quit\n"); 
				break;
			}
	  
			i++;
		}
		
		#ifdef CHECK_DEADLINE
		avg_pos_jitter = (double)(total_pos_jitter)/(50.0f-(double)missed_deadline);
		avg_neg_jitter = (double)(total_neg_jitter)/((double)missed_deadline);
		printf("\n\nAnalysis of Canny Edge Detection for 50 frames");
		printf("\nMissed Deadlines: %d", missed_deadline);
		printf("\nDeadlines Passed: %d", (50-missed_deadline));  
		printf("\nAverage positive jitter: %f", avg_pos_jitter);
		printf("\nAverage negative jitter: %f", avg_neg_jitter);
		#endif
		
		cvDestroyWindow("Canny Edge Detection");
		sem_post(&(cam[1]));
	}
}

int main()
{
	int rt_max_prio;
	int rc;
	int dev = 0;
	int i;
	pthread_t threads[NUM_THREADS]; //Thread descriptors
	pthread_attr_t rt_sched_attr[NUM_THREADS]; //Thread attributes object
	struct sched_param rt_param;
	struct sched_param main_param;

	sem_init(&(cam[0]), 0, 1); //Initialize semaphore 1
	sem_init(&(cam[1]), 0, 0); //Initialize semaphore 1
	sem_init(&(cam[2]), 0, 0); //Initialize semaphore 1
	
	capture = (CvCapture *)cvCreateCameraCapture(dev);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
		
	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	main_param.sched_priority=rt_max_prio; //Set scheduling priority as maximum (99)
   	sched_setscheduler(getpid(), SCHED_FIFO, &main_param); //Set scheduling policy as SCHED_FIFO

	rt_param.sched_priority=rt_max_prio-1; //Set priority of 98 to task 1

	for(i=0;i<NUM_THREADS;i++)
	{
		pthread_attr_init(&rt_sched_attr[i]);
		rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO); //Set SCHED_FIFO
        pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param);
	}

	rc = pthread_create(&threads[0],   // pointer to thread descriptor
                      &rt_sched_attr[0],     // thread attributes object                      
                      canny, // thread function entry point
                      (void *)0 // parameters to pass in
                     );

	  if (rc) //Error checking if thread 1 has been created
			perror("pthread create");

	/* Create Thread 2 */
	rc = pthread_create(&threads[1],   // pointer to thread descriptor
                      &rt_sched_attr[1],     // thread attributes object                      
                      hough, // thread function entry point
                      (void *)0 // parameters to pass in
                     );

	if (rc) //Error checking if thread 2 has been created
			perror("pthread create");

	rc = pthread_create(&threads[2],   // pointer to thread descriptor
                      &rt_sched_attr[2],     // thread attributes object                      
                      hough_circle, // thread function entry point
                      (void *)0 // parameters to pass in
                     );

	if (rc) //Error checking if thread 2 has been created
                perror("pthread create");

	for(i=0;i<NUM_THREADS;i++)
		pthread_join(threads[i], NULL);
		
	cvReleaseCapture(&capture);
		
	for(i=0;i<NUM_THREADS;i++)
	{
		if(pthread_attr_destroy(&rt_sched_attr[i]) != 0)
			perror("attr destroy");
	}

	return 0;
}
