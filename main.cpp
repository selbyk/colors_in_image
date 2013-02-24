#include <stdio.h>
#include <list>
//#include <boost/mpl/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

//Not included in boost libraries, downloaded from Adobe's svn
#include "../boost_1_53_0/boost/gil/extension/numeric/sampler.hpp"
#include "../boost_1_53_0/boost/gil/extension/numeric/resample.hpp"

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;

typedef boost::tuple<int, int, int> rgb;

struct image_rgb{
	rgb value;
	int occurences;
	int weight;
};

struct color{
	rgb value;
	string hex;
	string name;
};

typedef boost::tuple<rgb, int> weighted_rgb;
typedef image_rgb rgb_in_image;
typedef color color_def;

list<rgb_in_image> rgbList;

void insert_rgb(rgb_in_image rgb2){
	if( rgbList.empty() )
		rgbList.push_back( rgb2 );
	else {
		list<rgb_in_image>::iterator it = rgbList.begin();
		while( it != rgbList.end() && it->value < rgb2.value ) ++it;
		
		if( it->value < rgb2.value )
			rgbList.insert(it, rgb2);
		else{
			rgb2.occurences += it->occurences;
			rgb2.weight += it->weight;
			it = rgbList.erase(it);
			rgbList.insert(it, rgb2);
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

    rgb8_image_t img;
    rgb8_view_t imageView;
    
    jpeg_read_image( "test.jpg", img );
    
    rgb8_image_t resized;
    img.width() > img.height() ? resized.recreate(250,(250*(float)img.height()/img.width()))
			       : resized.recreate((250*(float)img.width()/img.height()),250);

    resize_view( const_view(img), view(resized), bilinear_sampler() );
    
    imageView = view( resized );
    
    //color_converted_view<rgb8_pixel_t>( imageView );
    
    int imageWidth = imageView.width(), imageHeight = imageView.height();
    
    cout << "Width: " << imageWidth << ", Height: " << imageHeight << endl;
    
    int x,y;
    rgb8_view_t::xy_locator Loc = imageView.xy_at(0,0);
    rgb8_pixel_t pixel;
    rgb_in_image currentRgb;
    
    for( y = 0; y < imageHeight/2; ++y ){
	//rgb8_view_t::x_iterator iv_it = imageView.row_begin(y);
	for( x = 0; x < imageWidth/2; ++x ){
		pixel = *Loc;
		//cout << "R: " << (int)at_c<0>(pixel) << " G: " << (int)at_c<1>(pixel) << " B: " << (int)at_c<2>(pixel) << endl;
		get<0>(currentRgb.value) = (int)at_c<0>(pixel);
		get<1>(currentRgb.value) = (int)at_c<1>(pixel);
		get<2>(currentRgb.value) = (int)at_c<2>(pixel);
		currentRgb.occurences = 1;
		currentRgb.weight = x+y;
		insert_rgb( currentRgb );
		++Loc.x();
	}
	for( x; x < imageWidth; ++x ){
		pixel = *Loc;
		//cout << "R: " << (int)at_c<0>(pixel) << " G: " << (int)at_c<1>(pixel) << " B: " << (int)at_c<2>(pixel) << endl;
		get<0>(currentRgb.value) = (int)at_c<0>(pixel);
		get<1>(currentRgb.value) = (int)at_c<1>(pixel);
		get<2>(currentRgb.value) = (int)at_c<2>(pixel);
		currentRgb.occurences = 1;
		currentRgb.weight = imageWidth-x+y;
		insert_rgb( currentRgb );
		++Loc.x();
	}
	Loc.x() -= imageWidth;
	Loc.y()++;
    }
    for( y; y < imageHeight; ++y ){
	//rgb8_view_t::x_iterator iv_it = imageView.row_begin(y);
	for( x = 0; x < imageWidth/2; ++x ){
		pixel = *Loc;
		//cout << "R: " << (int)at_c<0>(pixel) << " G: " << (int)at_c<1>(pixel) << " B: " << (int)at_c<2>(pixel) << endl;
		get<0>(currentRgb.value) = (int)at_c<0>(pixel);
		get<1>(currentRgb.value) = (int)at_c<1>(pixel);
		get<2>(currentRgb.value) = (int)at_c<2>(pixel);
		currentRgb.occurences = 1;
		currentRgb.weight = x+imageHeight-y;
		insert_rgb( currentRgb );
		++Loc.x();
	}
	for( x; x < imageWidth; ++x ){
		pixel = *Loc;
		//cout << "R: " << (int)at_c<0>(pixel) << " G: " << (int)at_c<1>(pixel) << " B: " << (int)at_c<2>(pixel) << endl;
		get<0>(currentRgb.value) = (int)at_c<0>(pixel);
		get<1>(currentRgb.value) = (int)at_c<1>(pixel);
		get<2>(currentRgb.value) = (int)at_c<2>(pixel);
		currentRgb.occurences = 1;
		currentRgb.weight = imageWidth-x+imageHeight-y;
		insert_rgb( currentRgb );
		++Loc.x();
	}
	Loc.x() -= imageWidth;
	Loc.y()++;
    }
    
	for( list<rgb_in_image>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		//rgb2 = get<0>(*it);
		cout << "R: " << get<0>(it->value) <<
			" G: " << get<1>(it->value) <<
			" B: " << get<2>(it->value) <<
			" Occurences: " << it->occurences <<
			" Weight: " << it->weight << endl;
	}
	
	list<color_def> colorDefinitions;
	FILE *fp;
	
	fp = fopen("colors.dat", "r+");
	
	color_def currentColor;
	int l;
	char *name = new char[20], *hex = new char[6];
	
	while(fscanf(fp, "%s %s %i %i %i %i", name, hex, &get<0>(currentColor.value),
					      &get<0>(currentColor.value), &get<0>(currentColor.value), &l ) == 6){
		currentColor.name = name;
		currentColor.hex = hex;
		colorDefinitions.push_back( currentColor );
	}
	
	for( list<color_def>::iterator it = colorDefinitions.begin(); it != colorDefinitions.end(); ++it ){
		//rgb2 = get<0>(*it);
		cout << "R: " << get<0>(it->value) <<
			" G: " << get<1>(it->value) <<
			" B: " << get<2>(it->value) <<
			" Name: " << it->name <<
			" Hex: #" << it->hex << endl;
	}

    // Save the image upside down, preserving its native color space and channel depth
    //jpeg_write_view( "out-dynamic_image.jpg", flipped_up_down_view( imageView ) );

    return 0;
}