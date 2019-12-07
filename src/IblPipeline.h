//
// Created by Srf on 2019/11/17.
//

#ifndef SANDBOX_IBLPIPELINE_H
#define SANDBOX_IBLPIPELINE_H

#include "Scene.h"
#include "IblPrePass.h"
#include "BasePipeline.h"

class IblPipeline : public BasePipeline {
public:
    explicit IblPipeline(Scene *scene);
    ~IblPipeline() override;
    void Init() override;
    void Draw() override;
    void Update(float deltaTime) override;

private:
    Scene *scene;
    Skybox *skybox;
    IblPrePass *prePass;
};

#endif //SANDBOX_IBLPIPELINE_H
