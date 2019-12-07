//
// Created by Srf on 2019/11/17.
//

#ifndef SANDBOX_SIMPLEPIPELINE_H
#define SANDBOX_SIMPLEPIPELINE_H

#include "BasePipeline.h"
#include "Scene.h"

class SimplePipeline : public BasePipeline {
public:
    explicit SimplePipeline(Scene *scene);
    ~SimplePipeline() override;
    void Init() override;
    void Draw() override;
    void Update(float deltaTime) override;

private:
    Scene *scene;
    BasePrePass *prePass;
};


#endif //SANDBOX_SIMPLEPIPELINE_H
