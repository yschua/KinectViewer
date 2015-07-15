#pragma once
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>

struct CameraParameters {
  glm::mat3 depthIntrinsic = glm::mat3(363.637311580360f, 0.0f, 260.162463407267f,
                                       0.0f, 364.345422491291f, 206.802454008941f,
                                       0.0f, 0.0f, 1.0f);
  glm::mat3 depthIntrinsicInv = glm::inverse(depthIntrinsic);
  glm::mat4 depthToWorld = glm::mat4(0.145446715205969f, -0.989127468335669f, -0.0217279640017811f, -73.5533740586220f,
                                     0.948518063850822f, 0.145653332196769f, -0.281244714383515f, 257.491272781619f,
                                     0.281351622679666f, 0.0202967535297334f, 0.959390069894239f, 712.587998080505f,
                                     0.0f, 0.0f, 0.0f, 1.0f);

  glm::mat3 colorIntrinsic = glm::mat3(1058.146130208319f, 0.0f, 932.278509427921f,
                                       0.0f, 1060.14186103165f, 528.407562114846f,
                                       0.0f, 0.0f, 1.0f);
  glm::mat3 colorIntrinsicInv = glm::inverse(colorIntrinsic);
  glm::mat4 colorToWorld = glm::mat4(0.143678430270863f, -0.989451097747080f, -0.0185211728029845f, -22.6858703660179f,
                                     0.947176636034766f, 0.142914047434264f, -0.287109726752370f, 257.310388529524f,
                                     0.286727970077501f, 0.0237086527043277f, 0.957718628283997f, 714.071540046624f,
                                     0.0f, 0.0f, 0.0f, 1.0f);
  glm::mat4 depthToColor = glm::inverse(colorToWorld) * depthToWorld;
};