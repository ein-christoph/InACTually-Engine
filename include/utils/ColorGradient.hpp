
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder/app/App.h"
#include "CinderOpenCV.h"


namespace act {
	namespace util {

		/**
		* @brief constructs a ColorGradient for easy use
		*/
		class ColorGradient {
		public:
			ColorGradient() {
				add(ci::ColorA::gray(0.0f, 0.0f), 0.0f);
				add(ci::ColorA::gray(0.0f, 0.0f), 1.0f);
			};
			~ColorGradient() {};

			/**
			* @brief adds a color to the gradient at position
			* @param color color to be added
			* @param position position [0.0f, 1.0f] of the color in the gradient
			*/
			void add(ci::ColorA color, float position) {
				colors[position] = color;
			};

			/**
			* @brief get the interpolated color
			* @param position [0.0f, 1.0f] of the color in the gradient
			* @return interpolated result as color
			*/
			ci::ColorA at(float position) {
				if (position <= 0.0f)
					return colors[0.0f];
				if (position >= 1.0f)
					return colors[1.0f];
				if (abs(position) == NAN)
					return colors[0.0f];

				ci::ColorA	lowerci::Color = colors[0.0f];
				ci::ColorA	higherColor;

				float	lowerValue = 0.0f;
				float	higherValue = 1.0f;

				for (auto& kv : colors) {
					if (kv.first > position) {
						higherValue = kv.first;
						higherci::Color = kv.second;
						//higherci::Color = ci::rgbToHsv(higherColor);

						break;
					}
					lowerValue = kv.first;
					lowerci::Color = kv.second;
				}

				float alpha = (position - lowerValue) / (higherValue - lowerValue);
				return ci::lerp(lowerColor, higherColor, alpha);
			}

			static std::shared_ptr<ColorGradient> create() {
				return std::make_shared<ColorGradient>();
			}

			static std::shared_ptr<ColorGradient> createStdGradient() {
				auto colorGrad = ColorGradient::create();
				colorGrad->add(ci::ColorA::black(), 0.0f);
				colorGrad->add(ci::ColorA(0.0f, 0.82f, 0.88f, 1.0f), 0.33f);
				colorGrad->add(ci::ColorA(1.0f, 0.0f, 0.26f, 1.0f), 0.66f);
				colorGrad->add(ci::ColorA::white(), 1.0f);
				return colorGrad;
			}

			static cv::UMat toColorMap(std::shared_ptr<ColorGradient> colorGradient) {
				cv::Mat colorMap = cv::Mat(256, 1, CV_8UC3, cv::Scalar(0));

				for (int c = 0; c < colorMap.rows; c++)
				{
					colorMap.row(c).setTo(ci::toOcv(colorGradient->at((float)c / 255.0f)));
				}
				cv::UMat uCM;
				colorMap.copyTo(uCM);
				return uCM;
			}

			static cv::UMat createStdColorMap() {
				return toColorMap(createStdGradient());
			}

		private:
			std::map<float, ci::ColorA> colors;
		};
		using ColorGradientRef = std::shared_ptr<ColorGradient>;

	}
}