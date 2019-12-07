//
// Created by Srf on 2019/11/17.
//

#ifndef SANDBOX_BASEPIPELINE_H
#define SANDBOX_BASEPIPELINE_H

class BasePipeline {
public:
    BasePipeline() = default;
    virtual ~BasePipeline() = default;
    virtual void Init() = 0;
    virtual void Draw() = 0;
    virtual void Update(float deltaTime) = 0;
};

#endif //SANDBOX_BASEPIPELINE_H
