#include <iostream>
#include <fstream>
#include <list>
#include <stdio.h>
#include <math.h>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/timer.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>
#include "../boost_1_53_0/boost/gil/extension/numeric/sampler.hpp" //Not included in boost libraries, downloaded from Adobe's svn
#include "../boost_1_53_0/boost/gil/extension/numeric/resample.hpp"

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;
using namespace boost::numeric::ublas;

//Configuration
const string imageNames[9] = {"oxford.jpg", "panda.jpg","red_dress.jpg","girl_shirt.jpg","lotus.jpg","flower_dress.jpg","flower.jpg","earth.jpg","amber_shirt.jpg"};
#define RESIZE_TO 450
#define THRESHOLD_MIN 2
#define THRESHOLD_MAX 256
#define NUM_VARIED_WEIGHTS 9
#define PERCENT_TO_TEST 0.60  //Center % of image to check
#define ENTROPY_THRESHOLD 70

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
	unsigned int entropy;
	long int varied_weights[NUM_VARIED_WEIGHTS];
	int avgX;
	int avgY;
} image_rgb;
typedef struct{
	list<int> entropies;
	int max;
	int min;
} region_entropies;
typedef struct{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
	long int varied_weights[NUM_VARIED_WEIGHTS];
	unsigned int entropy;
	string hex;
	string name;
	int avgX;
	int avgY;
} image_color;

//Image manipulation functions
void load_image(string filename);
void kernel_1d_convolution(matrix<int> kernel);
void kernel_2d_convolution(matrix<int> kernelX, matrix<int> kernelY, float divisor = 1);
void calculate_entropies();
void generate_entropy_squares();
void extract_rgb_values();
void sort_rgb_values();

//List insertion and sorting
void load_color_definitions();
void insert_image_rgb(image_rgb rgb2);
void insert_image_color(image_color color);
bool image_color_compare (image_color first, image_color second); // comparison for image_colors to sort colorsInImage

//GLOBAL VARIABLES
rgb8_image_t sourceImage;
gray8_image_t convolutedImage;
rgb8_image_t entropySquares;
region_entropies entropies;
list<image_rgb> rgbList;
list<image_color> colorsInImage;
list<color_def> colorDefinitions;

int main() {
	//Timer from boost to test performance
	boost::timer t;
	
	//Print to file
	ofstream myfile;
	myfile.open ("example.html");
	
	myfile << "<html>\n<head>\n<style>\nbody{font-size:11px;}\n.block{width:32px;height:32px;margin:-2px;margin-right:2px !important;float:left;clear:both;}\n" <<
		  ".item{width:950px; margin-left: auto; margin-right: auto; clear:both;}\n" <<
		  ".color{width:100%;padding: 2px; margin-bottom: 5px; border: 1px; clear:both;}\n" <<
		  "</style>\n</head>\n<body>\n";
	
	load_color_definitions();
	
	cout << "Colors loaded in " << t.elapsed() << "s\n";
	
	for( int i = 0; i < 9; ++i ){
		rgbList.clear();
		colorsInImage.clear();
		entropies.entropies.clear();
		entropies.max = 0;
		entropies.min = 999999;
	  
		//Load image
		load_image("test_images/"+imageNames[i]);
		
		/*
		//1D convolution
		matrix<int> kernel (3, 3);
		//Edge detect
		kernel(0,0) = 0; kernel(1,0) = -1; kernel(2,0) = 0; 
		kernel(0,1) = -1; kernel(1,1) = 4; kernel(2,1) = -1; 
		kernel(0,2) = 0; kernel(1,2) = -1; kernel(2,2) = 0;
		//Edge detect, alternative
		kernel(0,0) = 0; kernel(1,0) = 1; kernel(2,0) = 0; 
		kernel(0,1) = 1; kernel(1,1) = -4; kernel(2,1) = 1; 
		kernel(0,2) = 0; kernel(1,2) = 1; kernel(2,2) = 0;
		
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
		
		//Scharr's edge-detect
		kernelX(0,0) = 3; kernelX(1,0) = 0; kernelX(2,0) = -3; 
		kernelX(0,1) = 10; kernelX(1,1) = 0; kernelX(2,1) = -10; 
		kernelX(0,2) = 3; kernelX(1,2) = 0; kernelX(2,2) = -3; 
		
		kernelY(0,0) = 3; kernelY(1,0) = 10; kernelY(2,0) = 3; 
		kernelY(0,1) = 0; kernelY(1,1) = 0; kernelY(2,1) = 0; 
		kernelY(0,2) = -3; kernelY(1,2) = -10; kernelY(2,2) = -3; 
		
		//Prewitt edge-detect
		/*kernelX(0,0) = -1; kernelX(1,0) = 0; kernelX(2,0) = 1; 
		kernelX(0,1) = -1; kernelX(1,1) = 0; kernelX(2,1) = 1; 
		kernelX(0,2) = -1; kernelX(1,2) = 0; kernelX(2,2) = 1; 
		
		kernelY(0,0) = 1; kernelY(1,0) = 1; kernelY(2,0) = 1; 
		kernelY(0,1) = 0; kernelY(1,1) = 0; kernelY(2,1) = 0; 
		kernelY(0,2) = -1; kernelY(1,2) = -1; kernelY(2,2) = -1;*/
		
		kernel_2d_convolution( kernelX, kernelY, 11 ); //Creates a greyscale edge detect convolution

		calculate_entropies();
		//Test print
		for( list<int>::iterator it = entropies.entropies.begin(); it != entropies.entropies.end(); ++it ){
			//rgb2 = get<0>(*it);
			cout << "R: " << *it << endl;
		}
		cout << "Min: " << entropies.min << endl;
		cout << "Max: " << entropies.max << endl;
		
		//generate_entropy_squares();
		
		extract_rgb_values(); //Extracts and weights each pixel

		//Test print
		/*for( list<image_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
			//rgb2 = get<0>(*it);
			cout << "R: " << get<0>(it->value) <<
				" G: " << get<1>(it->value) <<
				" B: " << get<2>(it->value) <<
				" Occurences: " << it->occurences <<
				" Weight: " << it->weight << endl;
		}*/
		
		sort_rgb_values(); //Groups rgb values by their closest match in color_definitions
		
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
		jpeg_write_view(string("images/input")+(char)(i+49)+".jpg", view(sourceImage) );
		jpeg_write_view(string("images/input_convoluted")+(char)(i+49)+".jpg", view(convolutedImage) );
		jpeg_write_view(string("images/input_ent_sqs")+(char)(i+49)+".jpg", view(entropySquares) );
		
		myfile << "<div class = 'item'>\n";
		myfile << "<div class = 'image'>\n" <<
			  "<img src='images/input" << i+1 << ".jpg'>\n" <<
			  "<img src='images/input_convoluted" << i+1 << ".jpg'>\n" <<
			  "<img src='images/input_ent_sqs" << i+1 << ".jpg'>\n</div>\n";
		for( list<image_color>::iterator it = colorsInImage.begin(); it != colorsInImage.end(); ++it ){
			myfile << "<div class = 'color' style='border:solid 1px " << it->hex << ";'>\n" <<
				  "<div class = 'block' style='background-color:" << it->hex << ";'></div>\n" <<
				  "Name: " << it->name <<
				" * RGB: (" << get<0>(it->value) <<
				", " << get<1>(it->value) <<
				", " << get<2>(it->value) <<
				") * Hex: " << it->hex <<
				" * Occurences: " << it->occurences << 
				" * Average Position: (" << it->avgX << ", " << it->avgY << ")\n" <<
				//" </br> Weight: " << it->weight <<
				"</br># Neighboring Pixels Over Treshold: (" << it->varied_weights[0];
			for( int i = 1; i < NUM_VARIED_WEIGHTS; ++i )
				myfile << ", " << it->varied_weights[i];
			myfile << ") Total Entropy: " << it->entropy << "\n</div>\n";
		}
		myfile << "</div>\n";
	}
	myfile << "</body>\n</html>\n";
	myfile.close();

	cout << "Time elapsed: " << t.elapsed() << "s\n";

	return 0;
}

void load_image(string filename){
	rgb8_image_t loadedImage;
	
	jpeg_read_image( filename, loadedImage );

	//Perform resize if necessary
	if( RESIZE_TO != 0 && (loadedImage.width() > RESIZE_TO || loadedImage.height() > RESIZE_TO) ){
		loadedImage.width() > loadedImage.height() ? sourceImage.recreate(RESIZE_TO,(RESIZE_TO*(float)loadedImage.height()/loadedImage.width()))
							   : sourceImage.recreate((RESIZE_TO*(float)loadedImage.width()/loadedImage.height()),RESIZE_TO);
		resize_view( const_view(loadedImage), view(sourceImage), bilinear_sampler() );
	}else{
		sourceImage.recreate( loadedImage.dimensions() );
		copy_pixels( const_view(loadedImage), view(sourceImage) );
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
			
			//Get rid of horizontal and vertical edges
			/*float degrees;
			dstXValue != 0 ? degrees = atan ((float)dstYValue/dstXValue)*57.2957795 : degrees = 0;
			if( degrees < 0 )
				degrees += 180;
			if( degrees < 22.5 )
				degrees = 0;
			else if( degrees > 67.5 && degrees < 112.5 )
				degrees = 90;
			if( degrees == 0 || degrees == 90 )
				*dstLoc = 0;*/
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

void calculate_entropies(){
	gray8_view_t test = view(convolutedImage);

	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width(), imageHeight = sourceImage.height();
	
	int xSteps = imageWidth/10;
	int xRemainder = imageWidth%10;
	int ySteps = imageHeight/10;
	int yRemainder = imageHeight%10;
	
	float xMultiplier, yMultiplier;
	if( xRemainder != 0 )
		 xMultiplier = 10/xRemainder;
	if( yRemainder != 0 )
		yMultiplier = 10/yRemainder;
	
	int entropy;

	//Prepare to traverse pixels!
	int x,y, o, a, i, j;
	rgb8_pixel_t pixel;
	gray8_view_t::xy_locator testLoc;

	entropies.max = 0;
	entropies.min = 9999999;
	for( y = 0; y < ySteps; y++ ){
		for( x = 0; x < xSteps; x++ ){
			testLoc = test.xy_at(x*10+1,y*10+1);

			entropy = 0;
			for( o = 0; o < 10; ++o ){
				for( a = 0; a <= 10; ++a ){
					for( i = -1; i <= 1; ++i )
						for( j = -1; j <= 1; ++j )
							entropy += abs( *testLoc - (testLoc(i, j)) );
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}
			entropies.entropies.push_back( entropy );
			if( entropy > entropies.max )
				entropies.max = entropy;
			else if( entropy < entropies.min )
				entropies.min = entropy;
			
		}
		if( xRemainder != 0 ){
			testLoc = test.xy_at(xSteps*10+1,ySteps*10+1);
			entropy = 0;
			for( o = 0; o < 10; ++o ){
				for( a = 0; a < xRemainder; ++a ){
					for( i = -1; i <= 1; ++i )
						for( j = -1; j <= 1; ++j )
							entropy += abs( *testLoc - (testLoc(i, j)) );
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}
			entropy *= xMultiplier;
			entropies.entropies.push_back( entropy );
			if( entropy > entropies.max )
				entropies.max = entropy;
			else if( entropy < entropies.min )
				entropies.min = entropy;
		}
	}
	if( yRemainder != 0 )
		for( x = 0; x < xSteps; x++ ){
			testLoc = test.xy_at(x*10+1,ySteps*10+1);

			entropy = 0;
			for( o = 0; o < yRemainder; ++o ){
				for( a = 0; a <= 10; ++a ){
					for( i = -1; i <= 1; ++i )
						for( j = -1; j <= 1; ++j )
							entropy += abs( *testLoc - (testLoc(i, j)) );
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}
			entropy *= yMultiplier;
			entropies.entropies.push_back( entropy );
			if( entropy > entropies.max )
				entropies.max = entropy;
			else if( entropy < entropies.min )
				entropies.min = entropy;
		}
	if( xRemainder != 0 && yRemainder != 0 ){
		testLoc = test.xy_at(x*10+1,ySteps*10+1);

		entropy = 0;
		for( o = 0; o < yRemainder; ++o ){
			for( a = 0; a <= xRemainder; ++a ){
				for( i = -1; i <= 1; ++i )
					for( j = -1; j <= 1; ++j )
						entropy += abs( *testLoc - (testLoc(i, j)) );
				testLoc.x()++;
			}
			testLoc.x() -= 10;
			testLoc.y()++;
		}
		entropy *= yMultiplier *= xMultiplier;
		entropies.entropies.push_back( entropy );
		if( entropy > entropies.max )
			entropies.max = entropy;
		else if( entropy < entropies.min )
			entropies.min = entropy;
	}
}

void generate_entropy_squares(){
	entropySquares.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels( const_view(sourceImage), view(entropySquares) );
	
	rgb8_view_t test = view(entropySquares);  

	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width(), imageHeight = sourceImage.height();
	
	int xSteps = imageWidth/10;
	int xRemainder = imageWidth%10;
	int ySteps = imageHeight/10;
	int yRemainder = imageHeight%10;
	
	float xMultiplier, yMultiplier;
	if( xRemainder != 0 )
		 xMultiplier = 10/xRemainder;
	else
		xMultiplier = 0;
	if( yRemainder != 0 )
		yMultiplier = 10/yRemainder;
	else
		yMultiplier = 0;
	
	float entropyMultiplier = 510/entropies.max;
	
	int offset = ((float)entropies.max*entropyMultiplier+(float)entropies.min*entropyMultiplier)/2;
	int entropy;
	list<int>::iterator it = entropies.entropies.begin();
	//it != entropies.entropies.end();
	//Prepare to traverse pixels!
	int x,y, o, a;
	rgb8_view_t::xy_locator testLoc;
	
	rgb8s_pixel_t pixelColor;

	entropies.max = 0;
	entropies.min = 99999;
	for( y = 0; y < ySteps; y++ ){
		for( x = 0; x < xSteps; x++ ){
			testLoc = test.xy_at(x*10+1,y*10+1);
			
			entropy = (*it)*entropyMultiplier-offset;
			if( entropy < 0 ){
				at_c<0>(pixelColor) = abs( entropy );
				at_c<1>(pixelColor) = 0;
				at_c<2>(pixelColor) = 0;
			} else {
				at_c<0>(pixelColor) = entropy;
				at_c<1>(pixelColor) = entropy;
				at_c<2>(pixelColor) = entropy;
			}
			
			for( o = 0; o < 10; ++o ){
				for( a = 0; a < 10; ++a ){
					at_c<0>(*testLoc) = at_c<0>(pixelColor);
					at_c<1>(*testLoc) = at_c<1>(pixelColor);
					//at_c<2>(*testLoc) = at_c<2>(pixelColor);
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}
			it++;
		}
		if( xRemainder != 0 ){
			testLoc = test.xy_at(xSteps*10+1,ySteps*10+1);
			entropy = (*it)*entropyMultiplier-offset;
			/*if( entropy < 0 ){
				at_c<0>(pixelColor) = abs( entropy );
				at_c<1>(pixelColor) = 0;
				at_c<2>(pixelColor) = 0;
			} else {
				at_c<0>(pixelColor) = entropy;
				at_c<1>(pixelColor) = entropy;
				at_c<2>(pixelColor) = entropy;
			}
			for( o = 0; o < 10; ++o ){
				for( a = 0; a < xRemainder; ++a ){
					*testLoc = pixelColor;
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}*/
			it++;
		}
	}
	if( yRemainder != 0 )
		for( x = 0; x < xSteps; x++ ){
			testLoc = test.xy_at(x*10+1,ySteps*10+1);

			entropy = (*it)*entropyMultiplier-offset;
			/*if( entropy < 0 ){
				at_c<0>(pixelColor) = abs( entropy );
				at_c<1>(pixelColor) = 0;
				at_c<2>(pixelColor) = 0;
			} else {
				at_c<0>(pixelColor) = entropy;
				at_c<1>(pixelColor) = entropy;
				at_c<2>(pixelColor) = entropy;
			}
			for( o = 0; o < yRemainder; ++o ){
				for( a = 0; a <= 10; ++a ){
					*testLoc = pixelColor;
					testLoc.x()++;
				}
				testLoc.x() -= 10;
				testLoc.y()++;
			}*/
			it++;
		}
	if( xRemainder != 0 && yRemainder != 0 ){
		testLoc = test.xy_at(x*10+1,ySteps*10+1);

		entropy = (*it)*entropyMultiplier-offset;
		/*if( entropy < 0 ){
			at_c<0>(pixelColor) = abs( entropy );
			at_c<1>(pixelColor) = 0;
			at_c<2>(pixelColor) = 0;
		} else {
			at_c<0>(pixelColor) = entropy;
			at_c<1>(pixelColor) = entropy;
			at_c<2>(pixelColor) = entropy;
		}
		for( o = 0; o < yRemainder; ++o ){
			for( a = 0; a <= xRemainder; ++a ){
				*testLoc = pixelColor;
				testLoc.x()++;
			}
			testLoc.x() -= 10;
			testLoc.y()++;
		}*/
		it++;
	}
}

void extract_rgb_values(){
	rgb8_view_t src = view(sourceImage);
	gray8_view_t test = view(convolutedImage);

	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width()*PERCENT_TO_TEST, imageHeight = sourceImage.height()*PERCENT_TO_TEST;
	int xc = imageWidth/2, yc = imageHeight/2;
	int numPix = imageWidth*imageHeight;

	//Prepare to traverse pixels!
	int x,y, i, j;
	rgb8_pixel_t pixel;
	image_rgb currentRgb;
	rgb8_view_t::xy_locator srcLoc = src.xy_at(sourceImage.width()*((1-PERCENT_TO_TEST)/2),sourceImage.height()*((1-PERCENT_TO_TEST)/2));
	gray8_view_t::xy_locator testLoc = test.xy_at(sourceImage.width()*((1-PERCENT_TO_TEST)/2),sourceImage.height()*((1-PERCENT_TO_TEST)/2));

	for( y = imageHeight-1; y > 0; y-- ){
		for( x = 0; x < imageWidth-1; x++ ){
			//Calculate varied_weight from convolution
			for( i = 0; i < NUM_VARIED_WEIGHTS; ++i )
				currentRgb.varied_weights[i] = 0;
			
			currentRgb.varied_weight = 0;
			currentRgb.entropy = 0;
			for( i = -1; i <= 1; ++i )
				for( j = -1; j <= 1; ++j ){
					if( abs( *testLoc - (testLoc(i, j)) ) >= THRESHOLD_MIN && abs( *testLoc - (testLoc(i, j)) ) <= THRESHOLD_MAX )
						currentRgb.varied_weight++;
					currentRgb.entropy += abs( *testLoc - (testLoc(i, j)) );
				}
			if( currentRgb.entropy < ENTROPY_THRESHOLD )
				currentRgb.entropy = 0;
			currentRgb.varied_weights[currentRgb.varied_weight]++;

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
	
	delete str;  //Cleaning up after myself
	
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
		color.entropy = it->entropy;
		for( int i = 0; i < NUM_VARIED_WEIGHTS; ++i )
			color.varied_weights[i] = it->varied_weights[i];
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
			rgb.entropy += it->entropy;
			rgb.occurences += it->occurences;
			rgb.weight += it->weight;
			for( int i = 0; i < NUM_VARIED_WEIGHTS; ++i )
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
			color.entropy += it->entropy;
			for( int i = 0; i < NUM_VARIED_WEIGHTS; ++i )
				color.varied_weights[i] += it->varied_weights[i];
			it = colorsInImage.erase(it);
			colorsInImage.insert(it, color);
		}
	}
}

bool image_color_compare (image_color first, image_color second){
	long long int firstWeight = 0, secondWeight = 0;
	for( int i = 0; i < NUM_VARIED_WEIGHTS; ++i ){
		firstWeight += first.varied_weights[i]*i;
		secondWeight += second.varied_weights[i]*i;
	}
	
	//if( firstWeight > secondWeight ) return true;
	if( (first.entropy+firstWeight)/2 > (second.entropy+secondWeight)/2 ) return true;
	else return false;
}