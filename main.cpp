#include <stdio.h>
#include <math.h>
#include <list>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

//Not included in boost libraries, downloaded from Adobe's svn
#include "../boost_1_53_0/boost/gil/extension/numeric/sampler.hpp"
#include "../boost_1_53_0/boost/gil/extension/numeric/resample.hpp"

#include <boost/timer.hpp>

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;

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
} image_rgb;
typedef struct{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
	unsigned long int varied_weights[9];
	string hex;
	string name;
} image_color;

//Image manipulation functions
void load_image(string filename);
void convolute_image();

//List insertion and sorting
void insert_image_rgb(image_rgb rgb2);
void insert_image_color(image_color color);
bool image_color_compare (image_color first, image_color second); // comparison for image_colors to sort colorsInImage

//GLOBAL VARIABLES
rgb8_image_t sourceImage;
gray8_image_t convolutedImage;
list<image_rgb> rgbList;
list<image_color> colorsInImage;
list<color_def> colorDefinitions;

int main() {
	//Timer from boost to test performance
	boost::timer t;
	
	//Load image
	load_image("oxford.jpg");
	
	jpeg_write_view("input.jpg", view(sourceImage) );
	
	convolute_image();
	
	jpeg_write_view("input_convoluted.jpg", view(convolutedImage) );
	
	//Perform some commonly used operations so they aren't done too often
	int imageWidth = sourceImage.width(), imageHeight = sourceImage.height();
	int xc = imageWidth/2, yc = imageHeight/2;
	int numPix = imageWidth*imageHeight;
	
	rgb8_view_t src = view(sourceImage);
	gray8_view_t test = view(convolutedImage);
	
	//Prepare to traverse pixels!
	int x,y;
	rgb8_pixel_t pixel;
	image_rgb currentRgb;
	rgb8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator testLoc = test.xy_at(1,1);
	for( y = 0; y < imageHeight-1; ++y ){
		for( x = 0; x < imageWidth-1; ++x ){
			//Calculate varied_weight from convolution
			for( int i = 0; i < 9; ++i )
				currentRgb.varied_weights[i] = 0;
			currentRgb.varied_weight = 0;
			if( *testLoc != (testLoc(0, 1)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(1, 1)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(1, 0)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(1, -1)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(0,-1)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(-1,-1)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(-1, 0)) )
				currentRgb.varied_weight++;
			if( *testLoc != (testLoc(-1, 1)) )
				currentRgb.varied_weight++;
			currentRgb.varied_weights[currentRgb.varied_weight] = 1;
			pixel = *srcLoc;
			get<0>(currentRgb.value) = (int)at_c<0>(pixel);
			get<1>(currentRgb.value) = (int)at_c<1>(pixel);
			get<2>(currentRgb.value) = (int)at_c<2>(pixel);
			currentRgb.occurences = 1;
			//Eqn to calculate pixel weight... needs to be thought through more.
			//In addition to this, weight is divided by occurences in sort
			currentRgb.weight = numPix*(float)(1/(float)log(pow( xc-x, 2 ) + pow( yc-y, 2 )+2));
			insert_image_rgb( currentRgb );
			++srcLoc.x();
			++testLoc.x();
		}
		srcLoc.x() -= (imageWidth - 1);
		testLoc.x() -= (imageWidth - 1);
		srcLoc.y()++;
		testLoc.y()++;
	}
    
	//Test print
	/*for( list<image_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		//rgb2 = get<0>(*it);
		cout << "R: " << get<0>(it->value) <<
			" G: " << get<1>(it->value) <<
			" B: " << get<2>(it->value) <<
			" Occurences: " << it->occurences <<
			" Weight: " << it->weight << endl;
	}*/
	
	//Load color definitions from file, yo
	FILE *fp;
	fp = fopen("colorDefinitions.dat", "r+");
	
	color_def currentColor;
	char *str = new char[35];
	do{
		currentColor.name = fgets( str, 36, fp );
		currentColor.hex = fgets( str, 8, fp );
		if( fscanf(fp," %i %i %i ",&get<0>(currentColor.value), &get<1>(currentColor.value), &get<2>(currentColor.value) ) == 3 )
			colorDefinitions.push_back( currentColor );
		else
			break;
		//cout << currentColor.name << endl;
		//while( fgetc(fp) != '\n' && !feof(fp) ); //Go to next line
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

	//Match rgb values to color definitions with names, hex values, and all that good stuff
	//uses rgb values as 3d points, assuming the shortest distance between the points is the
	//closest color match.  I am still not sure, but it seems to work.  Can't find a better way, either.
	image_color color;
	int shortestDist, currentDist;
	for( list<image_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		shortestDist = 255;
		for( list<color_def>::iterator it2 = colorDefinitions.begin(); it2 != colorDefinitions.end(); ++it2 ){
			currentDist = sqrt( pow( get<0>(it2->value)-get<0>(it->value), 2 )
					  + pow( get<1>(it2->value)-get<1>(it->value), 2 )
					  + pow( get<2>(it2->value)-get<2>(it->value), 2 ) );
			if( currentDist < shortestDist ){
				shortestDist = currentDist;
				currentColor = *it2;
			}
		}
		color.value = currentColor.value;
		color.hex = currentColor.hex;
		color.name = currentColor.name;
		color.occurences = it->occurences;
		color.weight = it->weight;
		for( int i = 0; i < 9; ++i ){
			color.varied_weights[i] = 0;
			color.varied_weights[i] += it->varied_weights[i];
		}
		insert_image_color( color );
	}
	
	//What we've all been waiting for.  Outputs the colors within the image in order of dominance
	colorsInImage.sort( image_color_compare );
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
	}

	cout << "Time elapsed: " << t.elapsed() << "s\n";

	return 0;
}

void load_image(string filename){
	rgb8_image_t loadedImage;
	
	jpeg_read_image( filename, loadedImage );
	//Deminsion restraint to resize to, in px 
	int resizeTo = 500;
	
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

void convolute_image(){
	convolutedImage.recreate( sourceImage.dimensions() );
	//Create grayscale copy of original image
	copy_pixels(color_converted_view<gray8_pixel_t>(const_view(sourceImage)), view(convolutedImage));
	
	gray8_image_t grayscaleImage( sourceImage.width()+2, sourceImage.height()+2 );
	
	resize_view( const_view(convolutedImage), view(grayscaleImage), bilinear_sampler() );
	
	int xMax = sourceImage.width(), yMax = sourceImage.height();
	
	gray8_view_t src = view(grayscaleImage);
	gray8_view_t dst = view(convolutedImage);
	
	gray8_pixel_t pixel;
	gray8_view_t::xy_locator srcLoc = src.xy_at(1,1);
	gray8_view_t::xy_locator dstLoc = dst.xy_at(0,0);
	int x,y, dstValue;
	for( y = 0; y < yMax; ++y ){
		for( x = 0; x < xMax; ++x ){
			dstValue = 0;
			dstValue -= 4*(*srcLoc);
			dstValue += srcLoc(0,1);
			dstValue += srcLoc(1,0);
			dstValue += srcLoc(0,-1);
			dstValue += srcLoc(-1,0);
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
			rgb.occurences += it->occurences;
			rgb.weight += it->weight;
			for( int i = 0; i < 9; ++i )
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
			color.occurences += it->occurences;
			color.weight += it->weight;
			for( int i = 0; i < 9; ++i )
				color.varied_weights[i] += it->varied_weights[i];
			it = colorsInImage.erase(it);
			colorsInImage.insert(it, color);
		}
	}
}

bool image_color_compare (image_color first, image_color second)
{
	unsigned long long int firstWeight = 0, secondWeight = 0;
	for( int i = 0; i < 9; ++i ){
		firstWeight += first.varied_weights[i]*i;
		secondWeight += second.varied_weights[i]*i;
	}
	
	//if( firstWeight/first.occurences > secondWeight/second.occurences ) return true;
	if( firstWeight > secondWeight ) return true;
	else return false;
}