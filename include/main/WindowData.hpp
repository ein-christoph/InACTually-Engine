
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder/app/App.h"
#include "cinder/app/Window.h"
#include "cinder/Json.h"

#include "Design.hpp"
#include "UniqueIDBase.hpp"

#include "DrawableBase.hpp"

#include <algorithm>
#include <functional>


namespace act {
	
	class WindowData : public UniqueIDBase {
	public:
		
		WindowData() {};
		~WindowData() {};

		static ci::app::WindowRef createWindow(std::string title,
			const glm::ivec2& size = glm::ivec2(800, 600),
			bool fullScreen = false,
			bool borderless = false) {

			ci::app::WindowRef window = ci::app::App::get()->createWindow(ci::app::Window::Format().size(size));
			
			window->setTitle(title);
			window->setUserData(new WindowData());

			window->setFullScreen(fullScreen);
			window->setBorderless(borderless);

			return window;
		};

		virtual void update() {
			if (m_drawable)
				m_drawable->update();
		};

		virtual void draw() {
			ci::gl::clear(util::Design::backgroundColor());
			ci::gl::color(ci::Color::white());

			if(!m_isInitialized) {
				m_isInitialized = true;
				
				auto ctx = ci::gl::context();
				if (ctx->getProjectionMatrixStack().size() == 0) {
					ctx->getProjectionMatrixStack().push_back(glm::mat4(1.0f));
				}
				if (ctx->getModelMatrixStack().size() == 0) {
					ctx->getModelMatrixStack().push_back(glm::mat4(1.0f));
				}
				if (ctx->getViewMatrixStack().size() == 0) {
					ctx->getViewMatrixStack().push_back(glm::mat4(1.0f));
				}
				
				ci::app::getWindow()->emitResize();
				return;
			}

			ci::gl::pushMatrices();

			if (m_drawable)
				m_drawable->draw();

			ci::gl::popMatrices();
		};

		virtual void fileDrop(ci::app::FileDropEvent event) {};
		virtual void resize() const {};
		
		void setDrawable(DrawableBaseRef drawable) {
			m_drawable = drawable;
		};

		ci::gl::TextureRef getFullscreenTex() const {
			return m_fullscreenTex;
		}
		void setFullscreenTex(const ci::gl::TextureRef& tex) {
			m_fullscreenTex = tex;
		}

		bool isFullscreen() { return m_isFullscreen; }
		void setIsFullscreen(bool isFullscreen) { 
			m_isFullscreen = isFullscreen;
			if(m_isFullscreen)
				ci::app::AppBase::get()->hideCursor();
			else
				ci::app::AppBase::get()->showCursor();
		}


	private:
		DrawableBaseRef m_drawable;
		bool m_isInitialized = false;
		ci::gl::TextureRef m_fullscreenTex = nullptr;
		bool m_isFullscreen = false;
	};

}