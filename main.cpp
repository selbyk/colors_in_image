#include <stdio.h>
#include <math.h>
#include <list>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

#include <boost/numeric/ublas/matrix.hpp>

//Not included in boost libraries, downloaded from Adobe's svn
#include "../boost_1_53_0/boost/gil/extension/numeric/sampler.hpp"
#include "../boost_1_53_0/boost/gil/extension/numeric/resample.hpp"

#include <fstream>

#include <boost/timer.hpp>

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;
using namespace boost::numeric::ublas;

//Just defining some types
typedef boost::tuple<int, int, int> rgb;
typedef struct{
	rgb value;
	string hex;
	string name;
} color_def;
typedef struct{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
	int varied_weight;
	unsigned long int varied_weights[9];
	int avgX;
	int avgY;
} image_rgb;
typedef struct{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
	unsigned long int varied_weights[9];
	string hex;
	string name;
	int avgX;
	int avgY;
} image_color;

//Image manipulation functions
void load_image(string filename);
void kernel_1d_convolution(matrix<int> kernel);
void kernel_2d_convolution(matrix<int> kernelX, matrix<int> kernelY, float divisor = 1);
void convolve_image();
void sobel_image();
void sharr_image();
void extract_rgb_values();
void extract_rgb_values_minus_20();
void sort_rgb_values();

//List insertion and sorting
void load_color_definitions();
void insert_image_rgb(image_rgb rgb2);
void insert_image_color(image_color color);
bool image_color_compare (image_color first, image_color second); // comparison for image_colors to sort colorsInImage

//GLOBAL VARIABLES
rgb8_image_t sourceImage;
gray8_image_t convolutedImage;
list<image_rgb> rgbList;
list<image_color> colorsInImage;
list<color_def> colorDefinitions;
int resizeTo = 450;
int minThreshold = 2;
int maxThreshold = 256;
int numVariedWeights = 9;

int main() {
	//Timer from boost to test performance
	boost::timer t;
	
	load_color_definitions();
	
	//Print to file
	ofstream myfile;
	myfile.open ("example.html");
	
	myfile << "<html><head><style>body{font-size: 11px;}.block{width:32px;height:32px;margin: -2px; margin-right: 2px !important; float:left;clear:both;}\n" <<
		  ".item{width:950px; margin-left: auto; margin-right: auto; clear:both;}\n" <<
		  ".color{width:100%;padding: 2px; margin-bottom: 5px; border: 1px; clear:both;}\n" <<
		  "</style></head><body>\n";
	
	string imageNames[8];
	imageNames[0] = "oxford.jpg"; 
	imageNames[1] = "panda.jpg";
	imageNames[2] = "red_dress.jpg";
	imageNames[3] = "girl_shirt.jpg";
	imageNames[4] = "lotus.jpg";
	imageNames[5] = "flower_dress.jpg";
	imageNames[6] = "flower.jpg";
	imageNames[7] = "earth.jpg";
	
	for( int i = 0; i < 8; ++i ){
		rgbList.clear();
		colorsInImage.clear();
	  
		//Load image
		load_image(imageNames[i]);
		
		//convolve_image();
		//sobel_image();
		//sharr_image();
		/*matrix<int> kernel (3, 3);
		
		//Edge detect
		kernel(0,0) = 0; kernel(1,0) = -1; kernel(2,0) = 0; 
		kernel(0,1) = -1; kernel(1,1) = 4; kernel(2,1) = -1; 
		kernel(0,2) = 0; kernel(1,2) = -1; kernel(2,2) = 0; 
		
		kernel_1d_convolution( kernel );
		*/
		matrix<int> kernelX (3, 3);
		matrix<int> kernelY (3, 3);
		
		/*
		//Sobel edge-detect
		kernelX(0,0) = -1; kernelX(1,0) = 0; kernelX(2,0) = 1; 
		kernelX(0,1) = -2; kernelX(1,1) = 0; kernelX(2,1) = 2; 
		kernelX(0,2) = -1; kernelX(1,2) = 0; kernelX(2,2) = 1; 
		
		kernelY(0,0) = -1; kernelY(1,0) = -2; kernelY(2,0) = -1; 
		kernelY(0,1) = 0; kernelY(1,1) = 0; kernelY(2,1) = 0; 
		kernelY(0,2) = 1; kernelY(1,2) = 2; kernelY(2,2) = 1; 
		*/
		/*
		//Scharr's edge-detect
		kernelX(0,0) = 3; kernelX(1,0) = 0; kernelX(2,0) = -3; 
		kernelX(0,1) = 10; kernelX(1,1) = 0; kernelX(2,1) = -10; 
		kernelX(0,2) = 3; kernelX(1,2) = 0; kernelX(2,2) = -3; 
		
		kernelY(0,0) = 3; kernelY(1,0) = 10; kernelY(2,0) = 3; 
		kernelY(0,1) = 0; kernelY(1,1) = 0; kernelY(2,1) = 0; 
		kernelY(0,2) = -3; kernelY(1,2) = -10; kernelY(2,2) = -3; 
		*/
		//Prewitt edge-detect
		kernelX(0,0) = -1; kernelX(1,0) = 0; kernelX(2,0) = 1; 
		kernelX(0,1) = -1; kernelX(1,1) = 0; kernelX(2,1) = 1; 
		kernelX(0,2) = -1; kernelX(1,2) = 0; kernelX(2,2) = 1; 
		
		kernelY(0,0) = 1; kernelY(1,0) = 1; kernelY(2,0) = 1; 
		kernelY(0,1) = 0; kernelY(1,1) = 0; kernelY(2,1) = 0; 
		kernelY(0,2) = -1; kernelY(1,2) = -1; kernelY(2,2) = -1;
		
		
		kernel_2d_convolution( kernelX, kernelY );
				    
		//extract_rgb_values();
		extract_rgb_values_minus_20();
		
		//Test print
		/*for( list<image_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
			//rgb2 = get<0>(*it);
			cout << "R: " << get<0>(it->value) <<
				" G: " << get<1>(it->value) <<
				" B: " << get<2>(it->value) <<
				" Occurences: " << it->occurences <<
				" Weight: " << it->weight << endl;
		}*/
		
		sort_rgb_values();
		
		//What we've all been waiting for.  Outputs the colors within the image in order of dominance
		colorsInImage.sort( image_color_compare );
		/*
		for( list<image_color>::iterator it = colorsInImage.begin(); it != colorsInImage.end(); ++it ){
			cout << "Name: " << it->name <<
				" RGB: (" << get<0>(it->value) <<
				", " << get<1>(it->value) <<
				", " << get<2>(it->value) <<
				") Hex: " << it->hex <<
				" Occurences: " << it->occurences << 
				" Weight: " << it->weight <<
				" Varied Weights: (" << it->varied_weights[0];
			for( int i = 1; i < 9; ++i )
				cout << ", " << it->varied_weights[i];
			cout << ")\n";
		}*/
		
		jpeg_write_view(string("input")+(char)(i+49)+".jpg", view(sourceImage) );
		jpeg_write_view(string("input_convoluted")+(char)(i+49)+".jpg", view(convolutedImage) );
		
		myfile << "<div class = 'item'>\n";
		myfile << "<div class = 'image'>\n" <<
			  "<img src='input" << i+1 << ".jpg'>\n" <<
			  "<img src='input_convoluted" << i+1 << ".jpg'>\n</div>\n";
		
		/*for( list<image_color>::iterator it = colorsInImage.begin(); it != colorsInImage.end(); ++it ){
			myfile << "<div class = 'block' style='background-color:" << it->hex << ";'></div>\n";
		}*/	  
		int imageWidth = sourceImage.width(), imageHeight = sourceImage.height();
		int xc = imageWidth/2, yc = imageHeight/2;
		for( list<image_color>::iterator it = colorsInImage.begin(); it != colorsInImage.end(); ++it ){
			myfile << "<div class = 'color' style='border:solid 1px " << it->hex << ";'>\n" <<
				  "<div class = 'block' style='background-color:" << it->hex << ";'></div>\n" <<
				  "Name: " << it->name <<
				" * RGB: (" << get<0>(it->value) <<
				", " << get<1>(it->value) <<
				", " << get<2>(it->value) <<
				") * Hex: " << it->hex <<
				" * Occurences: " << it->occurences << 
				" </br> Weight: " << it->weight <<
				"  * Varied Weights: (" << it->varied_weights[0];
			for( int i = 1; i < numVariedWeights; ++i )
				myfile << ", " << it->varied_weights[i];
			myfile << ") * Average Position: (" << it->avgX << ", " << it->avgY << ") ->Dist from center: "<< sqrt(pow(it->avgX-xc,2)+pow(it->avgY-yc,2)) <<"\n</div>\n";
		}
		myfile << "</div>\n";
	}
	myfile << "</body></html>\n";
	myfile.close();

	cout << "Time elapsed: " << t.elapsed() << "s\n";

	return 0;
}

void load_image(string filename){
	rgb8_image_t loadedImage;
	
	jpeg_read_image( filename, loadedImage );
	//Deminsion restraint to resize to, in px 

	
	//Perform resize if necessary
	if( resizeTo != 0 && (loadedImage.width() > resizeTo || loadedImage.height() > resizeTo) ){
		loadedImage.width() > loadedImage.height() ? sourceImage.recreate(resizeTo,(resizeTo*(float)loadedImage.height()/loadedImage.width()))
							   : sourceImage.recreate((resizeTo*(float)loadedImage.width()/loadedImage.height()),resizeTo);
		resize_view( const_view(loadedImage), view(sourceImage), bilinear_sampler() );
	}else{
		sourceImage.recreate( loadedImage.dimensions() );
		copy_pixels( const_view(loadedImage), view(sourceImage) );
	}
}

void convolve_image(){
	convolutedImage.recreate( sourceImage.dimensions() );

	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>( const_view(sourceImage)), view(convolutedImage) );
	
	gray8_image_t grayscaleImage( sourceImage.width()+2, sourceImage.height()+2 );
	
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x,y, dstValue;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstValue = 0;
			dstValue += 4*(*srcLoc);
			dstValue -= srcLoc(0,1);
			dstValue -= srcLoc(1,0);
			dstValue -= srcLoc(0,-1);
			dstValue -= srcLoc(-1,0);
			if( dstValue < 0 )
				dstValue = 0;
			*dstLoc = dstValue;

			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= xMax;
		dstLoc.x() -= xMax;
		srcLoc.y()++;
		dstLoc.y()++;
	}
  /*
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>(const_view(sourceImage)), view(convolutedImage));
	
	gray8_image_t grayscaleImage( sourceImage.width()+4, sourceImage.height()+4 );
	
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	convolutedImage.recreate( sourceImage.width()+2, sourceImage.height()+2 );
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x,y, dstValue;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstValue = 0;
			dstValue += 8*(*srcLoc);
			dstValue -= srcLoc(-1,1);
			dstValue -= srcLoc(1,1);
			dstValue -= srcLoc(1,-1);
			dstValue -= srcLoc(-1,-1);
			dstValue -= srcLoc(0,1);
			dstValue -= srcLoc(1,0);
			dstValue -= srcLoc(0,-1);
			dstValue -= srcLoc(-1,0);
			if( dstValue < 0 )
				dstValue = 0;
			*dstLoc = dstValue;
			//cout << (int)*srcLoc << "=>" << dstValue << endl;
			//cout << (int)at_c<1>(pixel)<< endl;
			//cout << (int)at_c<2>(pixel) ;
			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= (xMax);
		dstLoc.x() -= (xMax);
		srcLoc.y()++;
		dstLoc.y()++;
	}*/
}

void sobel_image(){
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>(const_view(sourceImage)), view(convolutedImage));
	
	gray8_image_t grayscaleImage( sourceImage.width()+4, sourceImage.height()+4 );
	
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	convolutedImage.recreate( sourceImage.width()+2, sourceImage.height()+2 );
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x,y, dstX, dstY;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstX = 0;
			dstX += -1*srcLoc(-1,1); dstX += 0*srcLoc(0,1); dstX += srcLoc(1,1);
			dstX += -2*srcLoc(-1,0);  dstX += 0*(*srcLoc); dstX += 2*srcLoc(1,0);
			dstX += -1*srcLoc(-1,-1); dstX += 0*srcLoc(0,-1); dstX += srcLoc(1,-1);
			dstY = 0;
			dstY += -1*srcLoc(-1,1); dstY += -2*srcLoc(0,1); dstY += -1*srcLoc(1,1);
			dstY += 0*srcLoc(-1,0);  dstY += 0*(*srcLoc); dstY += 0*srcLoc(1,0);
			dstY += 1*srcLoc(-1,-1); dstY += 2*srcLoc(0,-1); dstY += 1*srcLoc(1,-1);
			*dstLoc = sqrt(pow(dstX,2)+pow(dstY,2));
			//cout << (int)*srcLoc << "=>" << dstValue << endl;
			//cout << (int)at_c<1>(pixel)<< endl;
			//cout << (int)at_c<2>(pixel) ;
			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= (xMax);
		dstLoc.x() -= (xMax);
		srcLoc.y()++;
		dstLoc.y()++;
	}
}

void sharr_image(){
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>(const_view(sourceImage)), view(convolutedImage));
	
	gray8_image_t grayscaleImage( sourceImage.width()+4, sourceImage.height()+4 );
	
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	convolutedImage.recreate( sourceImage.width()+2, sourceImage.height()+2 );
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x,y, dstX, dstY;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstX = 0;
			dstX += 3*srcLoc(-1,1); dstX += 0*srcLoc(0,1); dstX += -3*srcLoc(1,1);
			dstX += 10*srcLoc(-1,0);  dstX += 0*(*srcLoc); dstX += -10*srcLoc(1,0);
			dstX += 3*srcLoc(-1,-1); dstX += 0*srcLoc(0,-1); dstX += -3*srcLoc(1,-1);
			dstY = 0;
			dstY += 3*srcLoc(-1,1); dstY += 10*srcLoc(0,1); dstY += 3*srcLoc(1,1);
			dstY += 0*srcLoc(-1,0);  dstY += 0*(*srcLoc); dstY += 0*srcLoc(1,0);
			dstY += -3*srcLoc(-1,-1); dstY += -10*srcLoc(0,-1); dstY += -3*srcLoc(1,-1);
			*dstLoc = sqrt(pow(dstX,2)+pow(dstY,2))/13;
			//cout << (int)*srcLoc << "=>" << dstValue << endl;
			//cout << (int)at_c<1>(pixel)<< endl;
			//cout << (int)at_c<2>(pixel) ;
			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= (xMax);
		dstLoc.x() -= (xMax);
		srcLoc.y()++;
		dstLoc.y()++;
	}
}

void kernel_1d_convolution(matrix<int> kernel){
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>( const_view(sourceImage)), view(convolutedImage) );
	gray8_image_t grayscaleImage( sourceImage.width()+2, sourceImage.height()+2 );
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	int anchor = kernel.size1()/2;
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x, y, i, j, dstValue;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstValue = 0;
			for( j = anchor; j >= -1*anchor; j-- )
				for( i = -1*anchor; i <= anchor; i++ )
					dstValue += srcLoc(i,j)*kernel(i+anchor,-1*j+anchor);
			if( dstValue < 0 )
				dstValue = 0;
			*dstLoc = dstValue;
			
			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= xMax;
		dstLoc.x() -= xMax;
		srcLoc.y()++;
		dstLoc.y()++;
	}
}

void kernel_2d_convolution(matrix<int> kernelX, matrix<int> kernelY, float divisor){
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>( const_view(sourceImage)), view(convolutedImage) );
	gray8_image_t grayscaleImage( sourceImage.width()+2, sourceImage.height()+2 );
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	int anchor = kernelX.size1()/2;
	
	int xMax = convolutedImage.width(), yMax = convolutedImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x, y, i, j, dstXValue, dstYValue;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstXValue = 0;
			dstYValue = 0;
			for( j = anchor; j >= -1*anchor; j-- )
				for( i = -1*anchor; i <= anchor; i++ ){
					dstXValue += srcLoc(i,j)*kernelX(i+anchor,-1*j+anchor);
					dstYValue += srcLoc(i,j)*kernelY(i+anchor,-1*j+anchor);
				}
			*dstLoc = sqrt( pow(dstXValue, 2) + pow( dstYValue, 2) )/divisor;
			
			float degrees;
			dstXValue != 0 ? degrees = atan ((float)dstYValue/dstXValue)*57.2957795 : degrees = 0;
			if( degrees < 0 )
				degrees += 180;
			if( degrees < 22.5 )
				degrees = 0;
			else if( degrees < 67.5 )
				degrees = 45;
			else if( degrees < 112.5 )
				degrees = 90;
			else
				degrees = 135;
			
			if( degrees == 0 || degrees == 90 )
				*dstLoc = 0;
			//cout << atan ((float)dstYValue/dstXValue)*57.2957795 << endl;
			
			++srcLoc.x();
			++dstLoc.x();
		}
		srcLoc.x() -= xMax;
		dstLoc.x() -= xMax;
		srcLoc.y()++;
		dstLoc.y()++;
	}
}

void extract_rgb_values(){
	rgb8_view_t src = view(sourceImage);
	gray8_view_t test = view(convolutedImage);
	
	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width(), imageHeight = sourceImage.height();
	int xc = imageWidth/2, yc = imageHeight/2;
	int numPix = imageWidth*imageHeight;
	
	//Prepare to traverse pixels!
	int x,y;
	rgb8_pixel_t pixel;
	image_rgb currentRgb;
	rgb8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator testLoc = test.xy_at(1,1);

	for( y = imageHeight-1; y > 0; y-- ){
		for( x = 0; x < imageWidth-1; x++ ){
			//Calculate varied_weight from convolution
			for( int i = 0; i < numVariedWeights; ++i )
				currentRgb.varied_weights[i] = 0;
			currentRgb.varied_weight = 0;
			if( abs( *testLoc - (testLoc(0, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(0, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, -1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(0,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(0, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//step further
			/*
			if( abs( *testLoc - (testLoc(0, 2)) ) >= minThreshold && abs( *testLoc - (testLoc(0, 2)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(2, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(2, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(1, -1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, -1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(0,-2)) ) >= minThreshold && abs( *testLoc - (testLoc(0, -2)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(-1,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, -1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-2, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(-2, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(-1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			*/
			currentRgb.varied_weights[currentRgb.varied_weight]++;
				//currentRgb.varied_weights[currentRgb.varied_weight] += (*testLoc)/10;
			
			pixel = *srcLoc;
			get<0>(currentRgb.value) = (int)at_c<0>(pixel);
			get<1>(currentRgb.value) = (int)at_c<1>(pixel);
			get<2>(currentRgb.value) = (int)at_c<2>(pixel);
			currentRgb.occurences = 1;
			//Eqn to calculate pixel weight... needs to be thought through more.
			//In addition to this, weight is divided by occurences in sort
			currentRgb.weight = (float)numPix/sqrt(pow( xc-x, 2 ) + pow( yc-y, 2 )+2);
			currentRgb.avgX = x;
			currentRgb.avgY = y;
			insert_image_rgb( currentRgb );
			++srcLoc.x();
			++testLoc.x();
		}
		srcLoc.x() -= (imageWidth - 1);
		testLoc.x() -= (imageWidth - 1);
		srcLoc.y()++;
		testLoc.y()++;
	}
}

void extract_rgb_values_minus_20(){
	rgb8_view_t src = view(sourceImage);
	gray8_view_t test = view(convolutedImage);
	
	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width()*0.60, imageHeight = sourceImage.height()*0.60;
	int xc = imageWidth/2, yc = imageHeight/2;
	int numPix = imageWidth*imageHeight;
	
	//Prepare to traverse pixels!
	int x,y;
	rgb8_pixel_t pixel;
	image_rgb currentRgb;
	rgb8_view_t::xy_locator srcLoc = src.xy_at(sourceImage.width()*0.20,sourceImage.height()*0.20);
	gray8_view_t::xy_locator testLoc = test.xy_at(sourceImage.width()*0.20,sourceImage.height()*0.20);

	for( y = imageHeight-1; y > 0; y-- ){
		for( x = 0; x < imageWidth-1; x++ ){
			//Calculate varied_weight from convolution
			for( int i = 0; i < numVariedWeights; ++i )
				currentRgb.varied_weights[i] = 0;
			currentRgb.varied_weight = 0;
			if( abs( *testLoc - (testLoc(0, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(0, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(1, -1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(0,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(0, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, -1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 1)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//step further
			/*if( abs( *testLoc - (testLoc(0, 2)) ) >= minThreshold && abs( *testLoc - (testLoc(0, 2)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, 1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(2, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(2, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(1, -1)) ) >= minThreshold && abs( *testLoc - (testLoc(1, -1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(0,-2)) ) >= minThreshold && abs( *testLoc - (testLoc(0, -2)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(-1,-1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, -1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			if( abs( *testLoc - (testLoc(-2, 0)) ) >= minThreshold && abs( *testLoc - (testLoc(-2, 0)) ) <= maxThreshold )
				currentRgb.varied_weight++;
			//if( abs( *testLoc - (testLoc(-1, 1)) ) >= minThreshold && abs( *testLoc - (testLoc(-1, 1)) ) <= maxThreshold )
				//currentRgb.varied_weight++;
			*/
			currentRgb.varied_weights[currentRgb.varied_weight]++;
			//currentRgb.varied_weights[currentRgb.varied_weight] += (*testLoc)/10;
			
			pixel = *srcLoc;
			get<0>(currentRgb.value) = (int)at_c<0>(pixel);
			get<1>(currentRgb.value) = (int)at_c<1>(pixel);
			get<2>(currentRgb.value) = (int)at_c<2>(pixel);
			currentRgb.occurences = 1;
			//Eqn to calculate pixel weight... needs to be thought through more.
			//In addition to this, weight is divided by occurences in sort
			currentRgb.weight = (float)numPix/sqrt(pow( xc-x, 2 ) + pow( yc-y, 2 )+2);
			currentRgb.avgX = x;
			currentRgb.avgY = y;
			insert_image_rgb( currentRgb );
			++srcLoc.x();
			++testLoc.x();
		}
		srcLoc.x() -= (imageWidth - 1);
		testLoc.x() -= (imageWidth - 1);
		srcLoc.y()++;
		testLoc.y()++;
	}
}

void load_color_definitions(){
	//Load color definitions from file, yo
	FILE *fp;
	fp = fopen("colorDefinitions.dat", "r+");
	
	color_def currentColor;
	char *str = new char[35];
	do{
		currentColor.name = fgets( str, 36, fp );
		//currentColor.hex = fgets( str, 8, fp );
		if( fscanf(fp," %s %i %i %i ", str, &get<0>(currentColor.value), &get<1>(currentColor.value), &get<2>(currentColor.value) ) == 4 ){
			currentColor.hex = str;
			colorDefinitions.push_back( currentColor );
		}else
			break;
		//cout << currentColor.name << endl;
		while( fgetc(fp) != '\n' && !feof(fp) ); //Go to next line
	}while( !feof(fp) );
	/*while( fscanf(fp,"%s : %s : %i : %i : %i", name, hex, &get<0>(currentColor.value),
					      &get<1>(currentColor.value), &get<2>(currentColor.value) ) > 0 ){
		
		currentColor.name = name;
		currentColor.hex = hex;
		colorDefinitions.push_back( currentColor );
	cout << currentColor.name;
		
	}*/
	
	//Cleaning up after myself
	//delete name;
	delete str;
	
	//Test print
	/*for( list<color_def>::iterator it = colorDefinitions.begin(); it != colorDefinitions.end(); ++it ){
		cout << "R: " << get<0>(it->value) <<
			" G: " << get<1>(it->value) <<
			" B: " << get<2>(it->value) <<
			" Name: " << it->name <<
			" Hex: #" << it->hex << endl;
	}*/
}

void sort_rgb_values(){
	//Match rgb values to color definitions with names, hex values, and all that good stuff
	//uses rgb values as 3d points, assuming the shortest distance between the points is the
	//closest color match.  I am still not sure, but it seems to work.  Can't find a better way, either.
	image_color color;
	color_def currentColor;
	float shortestDist, currentDist;
	for( list<image_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		shortestDist = 99999999;
		for( list<color_def>::iterator it2 = colorDefinitions.begin(); it2 != colorDefinitions.end(); ++it2 ){
			/*currentDist = sqrt( pow( get<0>(it2->value)-get<0>(it->value), 2 )
					  + pow( get<1>(it2->value)-get<1>(it->value), 2 )
					  + pow( get<2>(it2->value)-get<2>(it->value), 2 ) );*/
			currentDist = (pow( get<0>(it2->value)-get<0>(it->value), 2 ))*0.299
				      + (pow( get<1>(it2->value)-get<1>(it->value), 2 ))*0.587
				      + (pow( get<2>(it2->value)-get<2>(it->value), 2 ))*0.114;
			if( currentDist <= shortestDist ){
				shortestDist = currentDist;
				currentColor = *it2;
			}
		}
		color.value = currentColor.value;
		color.hex = currentColor.hex;
		color.name = currentColor.name;
		color.occurences = it->occurences;
		color.weight = it->weight;
		color.avgX = it->avgX;
		color.avgY = it->avgY;
		for( int i = 0; i < numVariedWeights; ++i ){
			//color.varied_weights[i] = 0;
			color.varied_weights[i] = it->varied_weights[i];
		}
		insert_image_color( color );
	}
}

void insert_image_rgb(image_rgb rgb){
	if( rgbList.empty() )
		rgbList.push_back( rgb );
	else {
		list<image_rgb>::iterator it = rgbList.begin();
		while( it != rgbList.end() && it->value < rgb.value ) ++it;
		
		if( it->value < rgb.value || it == rgbList.end()){
			rgbList.insert(it, rgb);
		}else{
			rgb.avgX = (rgb.avgX*rgb.occurences+it->avgX*it->occurences)/(rgb.occurences+it->occurences);
			rgb.avgY = (rgb.avgY*rgb.occurences+it->avgY*it->occurences)/(rgb.occurences+it->occurences);
			rgb.occurences += it->occurences;
			rgb.weight += it->weight;
			for( int i = 0; i < numVariedWeights; ++i )
				rgb.varied_weights[i] += it->varied_weights[i];
			it = rgbList.erase(it);
			rgbList.insert(it, rgb);
		}
	}
}

void insert_image_color(image_color color){
	if( colorsInImage.empty() )
		colorsInImage.push_back( color );
	else {
		list<image_color>::iterator it = colorsInImage.begin();
		while( it != colorsInImage.end() && it->value < color.value ) ++it;
		
		if( it->value < color.value || it == colorsInImage.end())
			colorsInImage.insert(it, color);
		else{
			color.avgX = (color.avgX*color.occurences+it->avgX*it->occurences)/(color.occurences+it->occurences);
			color.avgY = (color.avgY*color.occurences+it->avgY*it->occurences)/(color.occurences+it->occurences);
			color.occurences += it->occurences;
			color.weight += it->weight;
			for( int i = 0; i < numVariedWeights; ++i )
				color.varied_weights[i] += it->varied_weights[i];
			it = colorsInImage.erase(it);
			colorsInImage.insert(it, color);
		}
	}
}

bool image_color_compare (image_color first, image_color second)
{
	unsigned long long int firstWeight = 0, secondWeight = 0;
	for( int i = 0; i < numVariedWeights; ++i ){
		firstWeight += first.varied_weights[i]*i;
		secondWeight += second.varied_weights[i]*i;
	}
	
	//if( firstWeight/first.occurences > secondWeight/second.occurences ) return true;
	if( firstWeight > secondWeight ) return true;
	else return false;
}