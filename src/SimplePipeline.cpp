//
// Created by Srf on 2019/11/17.
//

#include "SimplePipeline.h"

SimplePipeline::SimplePipeline(Scene *scene) {
    this->scene = scene;
    this->prePass = new BasePrePass();
}

SimplePipeline::~SimplePipeline() {
    delete prePass;
}

void SimplePipeline::Init() {
    scene->Init(this->prePass);
}

void SimplePipeline::Draw() {
    scene->Draw(this->prePass);
}

void SimplePipeline::Update(float deltaTime) {
    scene->Update(deltaTime);
}


