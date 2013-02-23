#include <list>
#include <boost/mpl/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

using namespace std;
using namespace boost::gil;
using namespace boost::tuples;

typedef boost::tuple<int, int, int> rgb;
typedef boost::tuple<rgb, int> weighted_rgb;

list<weighted_rgb> rgbList;

void insert_rgb(weighted_rgb rgb2){
	if( rgbList.empty() )
		rgbList.push_back( rgb2 );
	else {
		list<weighted_rgb>::iterator it = rgbList.begin();
		while( it != rgbList.end() && get<0>(*it) < get<0>(rgb2) ) ++it;
		
		if( get<0>(*it) < get<0>(rgb2) )
			rgbList.insert(it, rgb2);
		else{
			get<1>(rgb2) = get<1>(*it) + get<1>(rgb2);
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

    rgb8_image_t imageR;
    rgb8_view_t imageView;
    
    jpeg_read_image( "test.jpg", imageR );
    
    imageView = view( imageR );
    
    //color_converted_view<rgb8_pixel_t>( imageView );
    
    int imageWidth = imageView.width(), imageHeight = imageView.height();
    
    cout << "Width: " << imageWidth << ", Height: " << imageHeight << endl;
    
    int x,y;
    rgb8_view_t::xy_locator Loc = imageView.xy_at(0,0);
    rgb8_pixel_t pixel;
    rgb rgb2(0,0,0);
    weighted_rgb weightedRgb(rgb2, 0);
    for( y = 0; y < imageHeight; ++y ){
	//rgb8_view_t::x_iterator iv_it = imageView.row_begin(y);
	for( x = 0; x < imageWidth; ++x ){
		pixel = *Loc;
		//cout << "R: " << (int)at_c<0>(pixel) << " G: " << (int)at_c<1>(pixel) << " B: " << (int)at_c<2>(pixel) << endl;
		get<0>(rgb2) = (int)at_c<0>(pixel);
		get<1>(rgb2) = (int)at_c<1>(pixel);
		get<2>(rgb2) = (int)at_c<2>(pixel);
		get<0>(weightedRgb) = rgb2;
		get<1>(weightedRgb) = x+y;
		insert_rgb( weightedRgb );
		++Loc.x();
	}
	Loc.x() -= imageWidth;
	Loc.y()++;
    }
    
	for( list<weighted_rgb>::iterator it = rgbList.begin(); it != rgbList.end(); ++it ){
		rgb2 = get<0>(*it);
		cout << "R: " << get<0>(rgb2) <<
			" G: " << get<1>(rgb2) <<
			" B: " << get<2>(rgb2) <<
			"Weight: " << get<1>(*it) << endl;
	}

    // Save the image upside down, preserving its native color space and channel depth
    //jpeg_write_view( "out-dynamic_image.jpg", flipped_up_down_view( imageView ) );

    return 0;
}