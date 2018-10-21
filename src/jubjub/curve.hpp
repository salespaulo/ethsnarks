/*    
    copyright 2018 to the baby_jubjub_ecc Authors

    This file is part of baby_jubjub_ecc.

    baby_jubjub_ecc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    baby_jubjub_ecc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with baby_jubjub_ecc.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef JUBJUB_CURVE_HPP_
#define JUBJUB_CURVE_HPP_

#include <cassert>
#include <memory>

#include "ethsnarks.hpp"

namespace ethsnarks {


class jubjub_params {
public:
    const FieldT a;
    const FieldT d;

    jubjub_params() :
        a("168700"),
        d("168696")
    {}
};


class isOnCurve : public GadgetT {
private:
    /* no internal variables */
public:

    VariableT x;
    VariableT y;
    VariableT a;
    VariableT d;
    //intermeditate variables 
    VariableT xx;
    VariableT axx;
    VariableT dxx;
    VariableT yy;
    VariableT dxxyy;
    VariableT lhs;
    VariableT rhs;


    std::string annotation_prefix = "isonCurve";

    isOnCurve(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &x, const VariableT &y, 
                   const VariableT &a, const VariableT &d,
                   const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();

};


class FasterPointAddition : public GadgetT {
public:
    const jubjub_params &m_params;

    // First input point
    const VariableT m_X1;
    const VariableT m_Y1;

    // Second input point
    const VariableT m_X2;
    const VariableT m_Y2;

    // Intermediate variables
    const VariableT m_beta;
    const VariableT m_gamma;
    const VariableT m_delta;
    const VariableT m_epsilon;
    const VariableT m_tau;
    const VariableT m_X3;
    const VariableT m_Y3;

    FasterPointAddition(
        ProtoboardT &pb,
        const VariableT in_X1,
        const VariableT in_Y1,
        const VariableT in_X2,
        const VariableT in_Y2,
        const std::string &annotation_prefix
    ) :
        GadgetT(in_pb, annotation_prefix),
        m_X1(in_X1), m_Y1(in_Y1),
        m_X2(in_X2), m_Y2(in_Y2),
        m_beta(make_variable(in_pb, FMT(this->annotation_prefix, ".beta"))),
        m_gamma(make_variable(in_pb, FMT(this->annotation_prefix, ".gamma"))),
        m_delta(make_variable(in_pb, FMT(this->annotation_prefix, ".delta"))),
        m_epsilon(make_variable(in_pb, FMT(this->annotation_prefix, ".epsilon"))),
        m_tau(make_variable(in_pb, FMT(this->annotation_prefix, ".tau"))),
        m_X3(make_variable(in_pb, FMT(this->annotation_prefix, ".X3"))),
        m_Y3(make_variable(in_pb, FMT(this->annotation_prefix, ".Y3"))),
    {

    }

    void generate_r1cs_constraints()
    {
        this->pb.add_r1cs_constraint(
            ConstraintT(m_X1, m_Y2, m_beta),
                FMT(annotation_prefix, ".beta = X1 * Y2"));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_Y1, m_X2, m_gamma),
                FMT(annotation_prefix, ".gamma = Y1 * X2"));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_Y1, m_Y2, m_delta),
                FMT(annotation_prefix, ".delta = Y1 * Y2"));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_X1, m_X2, m_epsilon),
                FMT(annotation_prefix, ".epsilon = X1 * X2"));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_delta, m_delta, m_tau),
                FMT(annotation_prefix, ".tau = delta * epsilon"));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_X3, {1 + (m_params.d*m_tau)}, {m_beta + m_gamma}),
                FMT(annotation_prefix, ".x3 * (1 + (d*tau)) == (beta + gamma) "));

        this->pb.add_r1cs_constraint(
            ConstraintT(m_X3, {1 - (m_params.d*m_tau)}, {m_delta + (-jubjub_params.a * m_epsilon)}),
                FMT(annotation_prefix, ".x3 * (1 + (d*tau)) == (delta + (-a * epsilon))"));
    }

    void generate_r1cs_witness()
    {
        this->pb.val(m_beta) = this->pb.val(m_X1) * this->pb.val(m_Y2);
        this->pb.val(m_gamma) = this->pb.val(m_Y1) * this->pb.val(m_X2);
        this->pb.val(m_delta) = this->pb.val(m_Y1) * this->pb.val(m_Y2);
        this->pb.val(m_epsilon) = this->pb.val(m_X1) * this->pb.val(m_X2);
        this->pb.val(m_X3) = (this->pb.val(m_beta)+this->pb.val(m_gamma)) / (1 + (m_params.d * this->pb.val(m_tau)));
        this->pb.val(m_X3) = (this->pb.val(m_delta)+( -m_params.a * this->pb.val(m_epsilon))) / (1 - (m_params.d * this->pb.val(m_tau)));
    }
};


class pointAddition : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
public:
    VariableT a;
    VariableT d;

    std::shared_ptr<isOnCurve> jubjub_isOnCurve;

    // Inputs
    VariableT x1;
    VariableT y1;
    VariableT x2;
    VariableT y2;

    // Outputs
    VariableT x3;
    VariableT y3;

    //intermeditate variables
    VariableT x1x2;
    VariableT x1y2;
    VariableT y1y2;
    VariableT y1x2;
    VariableT x1x2y1y2;
    VariableT dx1x2y1y2;
    VariableT ax1x2;

    std::string annotation_prefix = "point Addition ";


    pointAddition(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,

                   const VariableT &x1, const VariableT &y1,
                   const VariableT &x2, const VariableT &y2,
                   const VariableT &x3, const VariableT &y3,

                   const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};


class conditionalPointAddition : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
public:
    VariableT a;
    VariableT d;


    //input variables 
    VariableT x1;
    VariableT y1;
    VariableT x2;
    VariableT y2;
    VariableT x3;
    VariableT y3;
    VariableT canAdd;

    //intermediate variables
    VariableT x_toAdd;
    VariableT y_toAdd;
    VariableT y_intermediate_toAdd1;
    VariableT y_intermediate_toAdd2;
    VariableT not_canAdd;


    std::string annotation_prefix = "conditioanl adiditon";

    std::shared_ptr<pointAddition> jubjub_pointAddition;


    conditionalPointAddition(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,
                   const VariableT &x1, const VariableT &y1,
                   const VariableT &x2, const VariableT &y2,
                   const VariableT &x3, const VariableT &y3,
                   const VariableT &canAdd, const std::string &_annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};


class pointMultiplication : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
public:
    VariableT a;
    VariableT d;

    VariableT x;
    VariableT y;

    VariableArrayT coef;

    // outputs
    VariableArrayT x_ret;
    VariableArrayT y_ret;

    int coef_size; //coeffient size

    //intermeditate variables
    VariableT x_zero;
    VariableT y_zero;


    std::shared_ptr<isOnCurve> jubjub_isOnCurve;

    // store the result of the current iteration
    VariableArrayT x_intermediary;
    VariableArrayT y_intermediary;


    std::vector<std::shared_ptr<pointAddition > > doub; //double
    std::vector<std::shared_ptr<conditionalPointAddition > > add; //double

    pointMultiplication(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,
                   const VariableT &x_base, const VariableT &y_base,
                   const VariableArrayT &coef, const VariableArrayT x_ret,
                   const VariableArrayT y_ret, const std::string &annotation_prefix, 
                   int coef_size);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};

// namespace ethsnarks
}

// JUBJUB_CURVE_HPP_
#endif
