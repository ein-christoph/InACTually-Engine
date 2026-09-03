
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

#include "stddef.hpp"


namespace act {
	namespace util {
		/**
		* @brief std definitions for Design related purposes
		*
		*/
		class Design {
		public:
			Design() {}
			~Design() {}

			/**
			* @brief darkPrimaryColor used for text-readability in highlighting and pop-outs
			* @param alpha opacity [0.0f, 1.0f]
			* @return dark tealy green
			*/
			static ci::ColorA darkPrimaryColor(float alpha = 1.0f) {
				return ci::ColorA(0.0f, 0.7059f, 0.4710f, alpha); // return ci::ColorA(0.0f, 0.8745f, 0.498f, alpha);
			}

			/**
			* @brief primaryColor used for highlighting and pop-outs
			* @param alpha opacity [0.0f, 1.0f]
			* @return tealy green
			*/
			static ci::ColorA primaryColor(float alpha = 1.0f) {
				return ci::ColorA(0.0f, 0.8745f, 0.498f, alpha);
			}
			/**
			* @brief highlightColor used for highlights
			* @param alpha opacity [0.0f, 1.0f]
			* @return tealy light green
			*/
			static ci::ColorA highlightColor(float alpha = 1.0f) {
				return ci::ColorA(0.1255f, 1.0f, 0.6235f, alpha);
			}
			/**
			* @brief errorci::Color used for highlighted errors
			* @param alpha opacity [0.0f, 1.0f]
			* @return soft darker red
			*/
			static ci::ColorA darkErrorColor(float alpha = 1.0f) {
				return ci::ColorA(0.9607f, 0.3451f, 0.3451f, alpha);
			}
			/**
			* @brief darkErrorci::Color used for errors
			* @param alpha opacity [0.0f, 1.0f]
			* @return soft light red
			*/
			static ci::ColorA errorColor(float alpha = 1.0f) {
				return ci::ColorA(0.9412f, 0.4118f, 0.4118f, alpha);
			}
			/**
			* @brief secondaryColor used for highlights and interactions if primary is already used
			* @param alpha opacity [0.0f, 1.0f]
			* @return dusty light blue
			*/
			static ci::ColorA secondaryColor(float alpha = 1.0f) {
				return ci::ColorA(0.0f, 0.8745f, 1.0f, alpha);
			}
			/**
			* @brief additionalci::Color used for exclamation or if primary and secondary is in use
			* @param alpha opacity [0.0f, 1.0f]
			* @return hot pink
			*/
			static ci::ColorA additionalColor(float alpha = 1.0f) {
				return ci::ColorA(0.8745f, 0.0f, 0.31175f, alpha);
			}
			/**
			* @brief grayci::Color used for decent figures on the backgroundColor
			* @param alpha opacity [0.0f, 1.0f]
			* @return normalo middle gray
			*/
			static ci::ColorA grayColor(float alpha = 1.0f) {
				return ci::ColorA::gray(0.5f, alpha);
			}
			/**
			* @brief dark gray as backgroundci::Color used for backgrounds
			* @param alpha opacity [0.0f, 1.0f]
			* @return dark bluish gray
			*/
			static ci::ColorA backgroundColor(float alpha = 1.0) {
				return ci::ColorA(0.06f, 0.06f, 0.1f, alpha);
			}

			/**
			* @brief defines a std gradient from black to secondaryColor to primaryColor to white
			* @return shared_ptr<ColorGradient>
			*/
			static std::shared_ptr<ColorGradient> gradient() {
				auto colorGrad = ColorGradient::create();
				colorGrad->add(ci::ColorA::black(), 0.0f);
				colorGrad->add(util::Design::secondaryColor(), 0.33f);
				colorGrad->add(util::Design::primaryColor(), 0.66f);
				colorGrad->add(ci::ColorA::white(), 1.0f);
				return colorGrad;
			}
			/**
			* @brief padding between panels
			* @return padding in x and y as glm::ivec2
			*/
			static glm::ivec2 padding() {
				return glm::ivec2(5);
			}
		};
	}
}