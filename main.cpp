#include <stdio.h>
#include <math.h>
#include <list>
//#include <boost/mpl/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

#include <boost/timer.hpp>

//Not included in boost libraries, downloaded from Adobe's svn
#include "../boost_1_53_0/boost/gil/extension/numeric/sampler.hpp"
#include "../boost_1_53_0/boost/gil/extension/numeric/resample.hpp"

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;

typedef boost::tuple<int, int, int> rgb;

struct image_rgb{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
};

struct color{
	rgb value;
	string hex;
	string name;
};

struct color_in_image{
	rgb value;
	unsigned int occurences;
	unsigned int weight;
	string hex;
	string name;
};

typedef boost::tuple<rgb, int> weighted_rgb;
typedef image_rgb rgb_in_image;
typedef color color_def;
typedef color_in_image image_color;

list<rgb_in_image> rgbList;
list<image_color> colorsInImage;

// comparison, not case sensitive.
bool compare_by_weight (image_color first, image_color second)
{
	if (first.weight/first.occurences > second.weight/second.occurences) return true;
	else return false;
}

void insert_rgb(rgb_in_image rgb2){
	if( rgbList.empty() )
		rgbList.push_back( rgb2 );
	else {
		list<rgb_in_image>::iterator it = rgbList.begin();
		while( it != rgbList.end() && it->value < rgb2.value ) ++it;
		
		if( it->value < rgb2.value || it == rgbList.end())
			rgbList.insert(it, rgb2);
		else{
			rgb2.occurences += it->occurences;
			rgb2.weight += it->weight;
			it = rgbList.erase(it);
			rgbList.insert(it, rgb2);
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
			it = colorsInImage.erase(it);
			colorsInImage.insert(it, color);
		}
	}
}
//typedef boost::mpl::vector<gray8_image_t, rgb8_image_t, gray16_image_t, rgb16_image_t> my_images_t;
//typedef any_image_view<typename detail::images_get_const_views_t<my_images_t>::type> const_view_t;

int main() {
	/* any_image<my_images_t> image;
	const_view_t view;
	
	jpeg_read_image( "test.jpg", image );
	view = const_view( image );
	color_converted_view<rgb8_pixel_t>(view);
	

	// Save the image upside down, preserving its native color space and channel depth
	jpeg_write_view( "test.jpg", view  );*/
      
	//Timer from boost to test performance
	boost::timer t;

	rgb8_image_t img;
	rgb8_view_t imageView;
	
	jpeg_read_image( "panda.jpg", img );
	
	rgb8_image_t resized;
	
	//Perform resize if necessary
	(img.width() > 500 || img.height() > 500) && img.width() > img.height()
		    ? resized.recreate(250,(250*(float)img.height()/img.width()))
		    : resized.recreate((250*(float)img.width()/img.height()),250);

	resize_view( const_view(img), view(resized), bilinear_sampler() );
	
	imageView = view( resized );
	
	//Perform some commonly used operations so they aren't done too often
	int imageWidth = imageView.width(), imageHeight = imageView.height();
	int xc = imageWidth/2, yc = imageHeight/2;
	int numPix = imageWidth*imageHeight;
	
	//Prepare to traverse pixels!
	int x,y;
	rgb8_pixel_t pixel;
	rgb_in_image currentRgb;
	rgb8_view_t::xy_locator Loc = imageView.xy_at(0,0);
	for( y = 0; y < imageHeight; ++y ){
		//rgb8_view_t::x_iterator iv_it = imageView.row_begin(y);
		for( x = 0; x < imageWidth; ++x ){
			pixel = *Loc;
			get<0>(currentRgb.value) = (int)at_c<0>(pixel);
			get<1>(currentRgb.value) = (int)at_c<1>(pixel);
			get<2>(currentRgb.value) = (int)at_c<2>(pixel);
			currentRgb.occurences = 1;
			currentRgb.weight = numPix*(float)(1/(float)log(pow( xc-x, 2 ) + pow( yc-y, 2 )));
			insert_rgb( currentRgb );
			++Loc.x();
		}
		Loc.x() -= imageWidth;
		Loc.y()++;
	}
    
	/*for( list<rgb_in_image>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		//rgb2 = get<0>(*it);
		cout << "R: " << get<0>(it->value) <<
			" G: " << get<1>(it->value) <<
			" B: " << get<2>(it->value) <<
			" Occurences: " << it->occurences <<
			" Weight: " << it->weight << endl;
	}*/
	
	list<color_def> colorDefinitions;
	
	//Load color definitions from file, yo
	FILE *fp;
	fp = fopen("colors.dat", "r+");
	
	color_def currentColor;
	
	// l is only here because i have been too lazy to alter color.dat.  it's a pain
	int l;
	char *name = new char[20], *hex = new char[6];
	
	while(fscanf(fp, "%s %s %i %i %i %i", name, hex, &get<0>(currentColor.value),
					      &get<1>(currentColor.value), &get<2>(currentColor.value), &l ) == 6){
		currentColor.name = name;
		currentColor.hex = hex;
		colorDefinitions.push_back( currentColor );
	}
	
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
	for( list<rgb_in_image>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
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
		insert_image_color( color );
	}
	
	//What we've all been waiting for.  Outputs the colors within the image in order of dominance
	colorsInImage.sort( compare_by_weight );
	for( list<image_color>::iterator it = colorsInImage.begin(); it != colorsInImage.end(); ++it ){
		cout << "Name: " << it->name <<
			" RGB: (" << get<0>(it->value) <<
			", " << get<1>(it->value) <<
			", " << get<2>(it->value) <<
			") Hex: #" << it->hex <<
			" Occurences: " << it->occurences << 
			" Weight: " << it->weight << endl;
	}
	
	cout << "Time elapsed: " << t.elapsed() << "s\n";

	return 0;
}