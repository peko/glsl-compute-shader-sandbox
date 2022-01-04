#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H
#include "glad/gl.h"
#include "spdlog/spdlog.h"
//
#include "texture.h"

namespace gcss {

class FrameBuffer {
 private:
  GLuint framebuffer;
  std::vector<GLenum> attachments;

 public:
  FrameBuffer(const std::vector<GLenum> attachments)
      : attachments(attachments) {
    glCreateFramebuffers(1, &framebuffer);
    glNamedFramebufferDrawBuffers(framebuffer, this->attachments.size(),
                                  this->attachments.data());

    spdlog::info("[FrameBuffer] framebuffer {:x} created", framebuffer);
  }

  void destroy() {
    spdlog::info("[FrameBuffer] framebuffer {:x} deleted", framebuffer);

    glDeleteFramebuffers(1, &framebuffer);
    this->framebuffer = 0;
  }

  void bindTexture(const Texture& texture, std::size_t attachment_index) const {
    glNamedFramebufferTexture(framebuffer, attachments.at(attachment_index),
                              texture.getTextureName(), 0);
  }

  void activate() const { glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); }

  void deactivate() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

}  // namespace gcss

#endif