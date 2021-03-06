//
// Created by salmon on 17-9-5.
//

#ifndef SIMPLA_SIMPLETIMEINTEGRATOR_H
#define SIMPLA_SIMPLETIMEINTEGRATOR_H

#include "simpla/engine/TimeIntegrator.h"
namespace simpla {
class SimpleTimeIntegrator : public engine::TimeIntegrator {
    SP_ENABLE_NEW_HEAD(engine::TimeIntegrator, SimpleTimeIntegrator);

   public:
    void DoSetUp() override;
    void DoUpdate() override;
    void DoTearDown() override;
    void Synchronize(int) override;
    void Advance(Real time_now, Real time_dt) override;
};
}  // namespace simpla
#endif  // SIMPLA_SIMPLETIMEINTEGRATOR_H
