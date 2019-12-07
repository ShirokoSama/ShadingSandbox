//
// Created by Srf on 2019/11/17.
//

#include "IblPipeline.h"
#include "IblPrePass.h"

IblPipeline::IblPipeline(Scene *scene) {
    this->scene = scene;
    this->skybox = new Skybox("resource/hdr/newport_loft.hdr");
    this->prePass = new IblPrePass(skybox->getEnvironmentCubeMap());
}

IblPipeline::~IblPipeline() {
    delete skybox;
    delete prePass;
}

void IblPipeline::Init() {
    this->scene->Init(this->prePass);
    this->skybox->Init();
    this->prePass->DoPrePass();
}

void IblPipeline::Draw() {
    this->scene->Draw(this->prePass);
    this->skybox->Draw();
    //this->skybox->Draw(prePass->irradianceCubeMap);
}

void IblPipeline::Update(float deltaTime) {
    this->scene->Update(deltaTime);
}
