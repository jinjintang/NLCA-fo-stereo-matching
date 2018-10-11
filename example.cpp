#include "qx_basic.h"
#include "qx_ppm.h"
#include "qx_nonlocal_cost_aggregation.h"
#define QX_DEF_SIGMA							0.1//0.1

void stereo(char*filename_disparity_map,char*filename_left_image,char*filename_right_image,int max_disparity,bool use_post_processing)
{
	double sigma=QX_DEF_SIGMA;
	unsigned char***left,***right,**disparity; 
	int h,w;

	qx_image_size(filename_left_image,h,w);//obtain image size

	left=qx_allocu_3(h,w,3);//allocate memory
	right=qx_allocu_3(h,w,3);
	disparity=qx_allocu(h,w);
	qx_loadimage(filename_left_image, left[0][0], h, w);//load left image
	qx_loadimage(filename_right_image, right[0][0], h, w);//load right image
	qx_nonlocal_cost_aggregation m_nlca;//non-local cost aggregation class
	if(max_disparity>=100)m_nlca.init(h,w,100,sigma);//initialization
	else m_nlca.init(h, w, 100, sigma);//initialization
	m_nlca.m_tf.build_tree(left[0][0]);
	m_nlca.m_tf_right.build_tree(right[0][0]);
	m_nlca.max_disparity = max_disparity;

	qx_timer timer;//
	timer.start();
	int s;
	
	for (s = 0; s <=max_disparity - 100; s += 100) {
		m_nlca.matching_cost(s, left, right);//compute matching cost

		m_nlca.disparity(s, disparity, 0);//compute disparity
		m_nlca.disparity(s, disparity, 1);//compute disparity
	}
	
	if (max_disparity % 100) {
		m_nlca.m_nr_plane = max_disparity - s;
		qx_freed_4(m_nlca.m_buf_d3);
		m_nlca.m_buf_d3 = qx_allocd_4(5, h, w, m_nlca.m_nr_plane);
		m_nlca.matching_cost(s, left, right);//compute matching cost

		m_nlca.disparity(s, disparity, 0);//compute disparity
		m_nlca.disparity(s, disparity, 1);//compute disparity
	}
	
	for (s = 0; s <=max_disparity - 100; s += 100) {
		m_nlca.disparity(s, disparity, 2);//compute disparity
	}
	if (max_disparity % 100)
	m_nlca.disparity(s, disparity, 2);//compute disparity
	
	if(use_post_processing) timer.time_display("Non-local cost aggregation (including non-local post processing)");
	else timer.time_display("Non-local cost aggregation (excluding non-local post processing)");

	//for(int y=0;y<h;y++) for(int x=0;x<w;x++) disparity[y][x]*=256/max_disparity;//rescale the disparity map for visualization
	qx_saveimage(filename_disparity_map,disparity[0],h,w,1);//write the obtained disparity map to the harddrive


	qx_freeu_3(left); left=NULL;//free memory
	qx_freeu_3(right); right=NULL;
	qx_freeu(disparity); disparity=NULL;
}
int main(int argc,char*argv[])
{
	char*filename_disparity_map0 = "C:\\Users\\Administrator\\Pictures\\disparity\\machine0.ppm";
	char*filename_disparity_map = "C:\\Users\\Administrator\\Pictures\\disparity\\machine.ppm";
	char*filename_left_image = "C:\\Users\\Administrator\\Pictures\\disparity\\j1.ppm";
	char*filename_right_image = "C:\\Users\\Administrator\\Pictures\\disparity\\j2.ppm";
	int max_disparity = 200;
	//stereo(filename_disparity_map,filename_left_image,filename_right_image,max_disparity,false);//excluding non-local post processing
	stereo(filename_disparity_map,filename_left_image,filename_right_image,max_disparity,true);//including non-local post processing
	return(0);
}
