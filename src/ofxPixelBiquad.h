#pragma once

#include "ofMain.h"

/// \brief a time biquad filter for ofPixels with raw coefficients:
///
/// w[n] = x[n] + fb1*w[n-1]+ fb2*w[n-2]
/// y[n] = ff0*w[n] + ff1*w[n-1] + ff2*w[n-2]
///
/// ff0, ff1 and ff2 = feedforward section.
/// fb1 and fb2 = feedback section.
///
/// With fb1 and fb2 the filter can go unstable,
/// therefore it has a message for clearing the buffers.
///
/// Until now, only ofPixels (with any number of channels) are allowed as input,
/// the internal calculation, however, is done with floats.
///
/// You can choose between several methods for dealing with color values out of range:
///
/// If bWrapPositive is set true, positive colors larger than 'wrapRange' will wrap around 0 and 'wrapRange',
/// otherwise they will be clipped to 1.
/// If bWrapNegative is set true, negative colors will wrap around 0 and 'wrapRange',
/// otherwise they will be clipped to 0.
/// If bAbsValue is set to true, negative colors will become positive.
/// This happens before any wrapping, so bWrapNegative will be ignored.
///
/// You can also adjust the overall output gain before clipping/wrapping.
///
/// \returns ofPixels
///
/// The filter checks if the format (width, height or channels) of the input pixels has changed
/// and performs reallocation and clearing of all the buffers automatically.


class ofxPixelBiquad {
	protected:
        float *buf_1, *buf_2, *buf_w;
        float ff0, ff1, ff2, fb1, fb2, myGain;
        int myWidth;
        int myHeight;
        int myChannels;
		bool bAllocated;
        int wrapRange;
        bool bWrapPositive, bWrapNegative, bAbsValue;
        ofPixels outPixels;
    public:
        /// constructors
        ofxPixelBiquad();
        ofxPixelBiquad(int w, int h, int channels);
        /// destructor
        ~ofxPixelBiquad();

		/// allocate buffers (will be called automatically by ofxPixelBiquad::in() 
		/// on the first time or when dimensions change
        void allocate(int w, int h, int channels);
		/// ask if buffers are allocated
		bool isAllocated(){ return bAllocated;}
		
        /// send ofPixels and calculate new filter state
        void in(const ofPixels& inPixels);
        /// get current filter state as ofPixels
        const ofPixels& out();
		/// clear the filter
        void clearFilter();

        /// how to deal with positive numbers out of range:
        void setWrapPositive(bool mode) {bWrapPositive = mode; }
        /// switch between wrapping and clipping
        void setWrapNegative(bool mode) {bWrapNegative = mode; }
        /// switch between wrapping and clipping
        void setAbsValue(bool mode) {bAbsValue = mode; }
        /// set the wrapping range
        void setWrapRange(float range) {wrapRange = max(1, static_cast<int>(range*256.f + 0.5f)); }
        /// set all 5 coefficients
        void setCoeffs(float _ff0, float _ff1, float _ff2, float _fb1, float _fb2) {ff0 = _ff0; ff1 = _ff1; ff2 = _ff2; fb1 = _fb1; fb2 = _fb2; }
        void setCoeffs(const vector<float>& coeffs);
        /// set individual coefficients
        void setFf0(float _ff0) { ff0 = _ff0; }
        void setFf1(float _ff1) { ff1 = _ff1; }
        void setFf2(float _ff2) { ff2 = _ff2; }
        void setFb1(float _fb1) { fb1 = _fb1; }
        void setFb2(float _fb2) { fb2 = _fb2; }
        /// set overall output gain
        void setGain(float gain) { myGain = gain; }

};

