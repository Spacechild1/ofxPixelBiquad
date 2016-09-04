#include "ofxPixelBiquad.h"

ofxPixelBiquad::ofxPixelBiquad() {
    /// default constructor
    ff0 = 1;
    ff1 = 0;
    ff2 = 0;
    fb1 = 0;
    fb2 = 0;
    myGain = 1;

    wrapRange = 256;
    bWrapPositive = false;
    bWrapNegative = false;
    bAbsValue = false;

	bAllocated = false;
    buf_1 = nullptr;
    buf_2 = nullptr;
    buf_w = nullptr;

    myWidth = 0;
    myHeight = 0;
    myChannels = 0;

}

ofxPixelBiquad::ofxPixelBiquad(int w, int h, int channels) {
    /// constructor with arguments. pixels are allocated and initialized with zeros.
    ff0 = 1;
    ff1 = 0;
    ff2 = 0;
    fb1 = 0;
    fb2 = 0;
    myGain = 1;

    wrapRange = 256;
    bWrapPositive = false;
    bWrapNegative = false;
    bAbsValue = false;

	bAllocated = false;
	buf_1 = nullptr;
	buf_2 = nullptr;
	buf_w = nullptr;
    
	allocate(w, h, channels);
}

ofxPixelBiquad::~ofxPixelBiquad() {
    /// destructor which deletes the buffers (but only if some allocation has already happened)
    // this check is actually redundant as deleting a nullptr will be ignored anyway
    // if there hasn't been any successful allocation, all three pointers still point to null.

    if (bAllocated) {
        delete[] buf_1;
        delete[] buf_2;
        delete[] buf_w;
    }
}

void ofxPixelBiquad::allocate(int w, int h, int channels){
    w = std::max(0, w);
    h = std::max(0, h);
	channels = std::max(0, channels);
	
	uint32_t size = w * h * channels;
	
	// check for bad dimensions
	if (size == 0 || channels > 4){
		cout << "bad dimensions, not allocating!\n";
		return;
	}
		
	if (bAllocated){
		delete[] buf_1;
		delete[] buf_2;
		delete[] buf_w;
    }
	
	buf_1 = new float[size];
	buf_2 = new float[size];
	buf_w = new float[size];

	outPixels.allocate(w, h, channels);
	
	myWidth = w;
	myHeight = h;
	myChannels = channels;
	bAllocated = true;
	
	clearFilter();
}


void ofxPixelBiquad::in(const ofPixels& inPixels) {
    if (!inPixels.isAllocated()){
        cout << "incoming pixels not allocated!\n";
        return;
    }

    const int w = inPixels.getWidth();
    const int h = inPixels.getHeight();
    const int channels = inPixels.getNumChannels();
	const uint32_t size = w*h*channels;

    /// check if the dimensions have changed and reallocate everything if necessary.

    if ((w != myWidth)||(h != myHeight)||(channels != myChannels)){
		allocate(w, h, channels);  
    }

	if (!bAllocated){
        cout << "error: allocation failed!\n"; // shouldn't happen!
        return;
    }

    /// calculate the following difference equation:
    ///
    /// w[n] = x[n] + fb1*w[n-1]+ fb2*w[n-2]
    /// y[n] = ff0*w[n] + ff1*w[n-1] + ff2*w[n-2]
    ///
    /// check for bWrapPositive, bWrapNegative and bAbsValue and perform clipping/wrapping accordingly.
	/// some code duplication, but less if-statements inside the for-loop
	
    const unsigned char* inPix = inPixels.getData();
    unsigned char* outPix = outPixels.getData();
    // generate tiny random offset to protect against denormals
    const float noise = ofRandom(-1e-006, 1e-006);

    if ((bWrapPositive == false) && (bWrapNegative == false) && (bAbsValue == false)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
             // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // clip the output
            temp2 = max(0, min(255, temp2));
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }
    else if ((bWrapPositive == true) && (bWrapNegative == false) && (bAbsValue == false)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
             // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // clip below 0 so negative values don't have to be computed
            temp2 = max(0, temp2);
            // wrap around 0 and 'wrapRange'
            temp2 = temp2 % wrapRange;
            // clip the output
            temp2 = min(255, temp2);
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }
    else if ((bWrapPositive == false) && (bAbsValue == true)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
             // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // take absolute value
            temp2 = abs(temp2);
            // clip the output (min is enough because there are no negative numbers)
            temp2 = min(255, temp2);
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }
    else if ((bWrapPositive == true) && (bAbsValue == true)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
             // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // take absolute value
            temp2 = abs(temp2);
            // wrap around 0 and 'wrapRange'
            temp2 = temp2 % wrapRange;
            // clip the output (min is enough because there are no negative numbers)
            temp2 = min(255, temp2);
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }
    else if ((bWrapPositive == true) && (bWrapNegative == true) && (bAbsValue == false)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
             // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // wrap around 0 and 'wrapRange'
            temp2 = temp2 % wrapRange;
            // correct for negative numbers
            temp2 = (temp2 < 0) ? temp2 + wrapRange : temp2;
            // clip the output (min is enough because there are no negative numbers)
            temp2 = min(255, temp2);
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }
    else if ((bWrapPositive == false) && (bWrapNegative == true) && (bAbsValue == false)){
        for (uint32_t i = 0; i < size; ++i){
            // limit w[n] to avoid overflow (1e+006 is arbitrary).
            buf_w[i] = max(-1.0e+006f, min(1.0e+006f, inPix[i]/255.0f + buf_1[i]*fb1 + buf_2[i]*fb2)) + noise;
            float temp1 = (buf_w[i]*ff0 + buf_1[i]*ff1 + buf_2[i]*ff2) * myGain;
            // round to int
            int temp2 = static_cast<int>(temp1*255.f + 0.5f);
            // wrap around 0 and 'wrapRange' only for negative numbers
            temp2 = (temp2 < 0) ? (temp2 % wrapRange) + wrapRange : temp2;
            // clip the output (min is enough because there are no negative numbers)
            temp2 = min(255, temp2);
            outPix[i] = static_cast<unsigned char>(temp2);
        }
    }

    /// move buffers by swapping the pointers.
    float * temp = buf_2;
    buf_2 = buf_1;
    buf_1 = buf_w;
    buf_w = temp;
    // buf_w points now to old buf_2 which will be overwritten the next time.
}


const ofPixels& ofxPixelBiquad::out() {
    /// returns ofPixels (no need for checking. in the worst case, outPixels are not allocated)

    return outPixels;
}


void ofxPixelBiquad::clearFilter() {
    /// clear the buffers (but only if the filter is not empty)
    if (bAllocated){
        const uint32_t size = myWidth*myHeight*myChannels;
        for (int32_t i = 0; i < size; ++i){
            buf_1[i] = 0;
            buf_2[i] = 0;
        }
    }
}

void ofxPixelBiquad::setCoeffs(const vector<float>& coeffs){
    if (coeffs.size() >= 5){
        ff0 = coeffs[0];
        ff1 = coeffs[1];
        ff2 = coeffs[2];
        fb1 = coeffs[3];
        fb2 = coeffs[4];
    } else {
        cout << "vector must have 5 elements!";
    }
}
