//
// Created by salmon on 16-11-4.
//
#include "Worker.h"
#include <set>
#include <simpla/mesh/MeshBlock.h>
#include <simpla/mesh/Attribute.h>

namespace simpla { namespace mesh
{


Worker::Worker() : m_chart_(c), m_model_(m) {}

Worker::~Worker() {};

std::ostream &Worker::print(std::ostream &os, int indent) const
{

    os << std::setw(indent + 1) << " " << " [" << get_class_name() << " : " << name() << "]" << std::endl;

    os << std::setw(indent + 1) << " " << "Config = {" << db << "}" << std::endl;

    if (m_chart_ != nullptr)
    {
        os << std::setw(indent + 1) << " " << "Chart = " << std::endl
           << std::setw(indent + 1) << " " << "{ " << std::endl;
        m_chart_->print(os, indent + 1);
        os << std::setw(indent + 1) << " " << "}," << std::endl;
    }
    return os;
}


void Worker::move_to(Patch const &p)
{
    post_process();
    auto m = p.mesh();
    auto id = m->id();
    m_chart_->move_to(p.mesh());
    for (auto &item:attributes()) { item->move_to(m, p.data(item->id())); }
    pre_process();
}

std::shared_ptr<model::Model> Worker::clone_model() const { return std::make_shared<model::Model>(); }

void Worker::deploy()
{
    concept::LifeControllable::deploy();
    if (m_chart_ == nullptr) { m_chart_ = clone_mesh(); }

    m_chart_->deploy();

    if (m_model_ == nullptr) { m_model_ = clone_model(); }

    m_model_->deploy();
}

void Worker::destroy()
{
    m_model_.reset();
    m_chart_.reset();
    concept::LifeControllable::destroy();
}

void Worker::pre_process()
{
    if (is_valid()) { return; } else { concept::LifeControllable::pre_process(); }
    ASSERT(m_chart_ != nullptr);
    m_chart_->pre_process();
    ASSERT(m_model_ != nullptr);
    m_model_->pre_process();
}

void Worker::post_process()
{
    ASSERT(m_chart_ != nullptr);
    m_model_->post_process();
    ASSERT(m_model_ != nullptr);
    m_chart_->post_process();
    if (!is_valid()) { return; } else { concept::LifeControllable::post_process(); }
}

void Worker::initialize(Real data_time, Real dt)
{
    pre_process();
    ASSERT (m_chart_ != nullptr);
    m_chart_->initialize(data_time, dt);
    m_model_->initialize(data_time, dt);
}

void Worker::finalize(Real data_time, Real dt)
{
//    next_phase(data_time, dt, max_phase_num() - current_phase_num());

    m_model_->finalize(data_time, dt);
    m_chart_->finalize(data_time, dt);
    post_process();
}

void Worker::sync() {}
//
//void Worker::phase(unsigned int num, Real data_time, Real dt)
//{
//    concept::LifeControllable::phase(num);
//    switch (num)
//    {
//        #define PHASE(_N_) case _N_: phase##_N_(data_time, dt); break;
//
//        PHASE(0);
//        PHASE(1);
//        PHASE(2);
//        PHASE(3);
//        PHASE(4);
//        PHASE(5);
//        PHASE(6);
//        PHASE(7);
//        PHASE(8);
//        PHASE(9);
//
//        #undef NEXT_PHASE
//        default:
//            break;
//    }
//}
//
//unsigned int Worker::next_phase(Real data_time, Real dt, unsigned int inc_phase)
//{
//    unsigned int start_phase = current_phase_num();
//    unsigned int end_phase = concept::LifeControllable::next_phase(inc_phase);
//
//    switch (start_phase)
//    {
//        #define NEXT_PHASE(_N_) case _N_: phase##_N_(data_time, dt);sync();++start_phase;if (start_phase >=end_phase )break;
//
//        NEXT_PHASE(0);
//        NEXT_PHASE(1);
//        NEXT_PHASE(2);
//        NEXT_PHASE(3);
//        NEXT_PHASE(4);
//        NEXT_PHASE(5);
//        NEXT_PHASE(6);
//        NEXT_PHASE(7);
//        NEXT_PHASE(8);
//        NEXT_PHASE(9);
//
//        #undef NEXT_PHASE
//        default:
//            break;
//    }
//    return end_phase;
//};


}}//namespace simpla { namespace mesh1
