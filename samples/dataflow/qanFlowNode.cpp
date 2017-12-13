/*
 Copyright (c) 2008-2017, Benoit AUTHEMAN All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author or Destrat.io nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library.
//
// \file	qanFaceNode.cpp
// \author	benoit@destrat.io
// \date	2017 12 12
//-----------------------------------------------------------------------------

// QuickQanava headers
#include "../../src/QuickQanava.h"
#include "./qanFlowNode.h"

namespace qan { // ::qan

void    FlowNodeBehaviour::inNodeInserted( qan::Node& inNode, qan::Edge& edge ) noexcept
{
    const auto inFlowNode = qobject_cast<qan::FlowNode*>(&inNode);
    const auto flowNodeHost = qobject_cast<qan::FlowNode*>(getHost());
    if ( inFlowNode != nullptr &&
         flowNodeHost != nullptr ) {
        //
        QObject::connect(inFlowNode,    &qan::FlowNode::outputChanged,
                         flowNodeHost,  &qan::FlowNode::inNodeOutputChanged);
    }
}

void    FlowNodeBehaviour::inNodeRemoved( qan::Node& inNode, qan::Edge& edge ) noexcept
{

}

QQmlComponent*  FlowNode::delegate(QQmlEngine& engine) noexcept
{
    static std::unique_ptr<QQmlComponent>   qan_FlowNode_delegate;
    if ( !qan_FlowNode_delegate )
        qan_FlowNode_delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/FlowNode.qml");
    return qan_FlowNode_delegate.get();
}

void    FlowNode::inNodeOutputChanged()
{
    qDebug() << "In node output value changed for " << getLabel();
}

void    FlowNode::setOutput(QVariant output) noexcept
{
    _output = output;
    emit outputChanged();
}

QQmlComponent*  PercentageNode::delegate(QQmlEngine& engine) noexcept
{
    static std::unique_ptr<QQmlComponent>   delegate;
    if ( !delegate )
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/PercentageNode.qml");
    return delegate.get();
}

QQmlComponent*  OperationNode::delegate(QQmlEngine& engine) noexcept
{
    static std::unique_ptr<QQmlComponent>   delegate;
    if ( !delegate )
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/OperationNode.qml");
    return delegate.get();
}

void    OperationNode::setOperation(Operation operation) noexcept
{
    if ( _operation != operation ) {
        _operation = operation;
        emit operationChanged();
    }
}

void    OperationNode::inNodeOutputChanged()
{
    FlowNode::inNodeOutputChanged();
    qreal o{0.}; // For the example sake we do not deal with overflow
    bool oIsInitialized{false};
    for ( const auto& inNode : getInNodes() ) {
        const auto inFlowNode = qobject_cast<qan::FlowNode*>(inNode.lock().get());
        if ( inFlowNode == nullptr ||
             !inFlowNode->getOutput().isValid())
            continue;
        bool ok{false};
        const auto inOutput = inFlowNode->getOutput().toReal(&ok);
        if ( ok ) {
            switch ( _operation ) {
            case Operation::Add:        o += inOutput; break;
            case Operation::Multiply:
                if ( !oIsInitialized ) {
                    o = inOutput;
                    oIsInitialized = true;
                } else
                    o *= inOutput;
                break;
            }
        }
    }
    setOutput(o);
}

qan::Node* FlowGraph::insertFlowNode(FlowNode::Type type)
{
    qan::Node* flowNode = nullptr;
    switch ( type ) {
    case qan::FlowNode::Type::Percentage:
        flowNode = insertNode<PercentageNode>(nullptr);
        insertPort(flowNode, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out, "OUT" );
        break;
    case qan::FlowNode::Type::Image:
        flowNode = insertNode<FlowNode>(nullptr);
        break;
    case qan::FlowNode::Type::Operation:
        flowNode = insertNode<OperationNode>(nullptr);
        // Insert out port first we need to modify it from OperationNode.qml delegate
        insertPort(flowNode, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out, "OUT" );
        insertPort(flowNode, qan::NodeItem::Dock::Left, qan::PortItem::Type::In, "IN" );
        insertPort(flowNode, qan::NodeItem::Dock::Left, qan::PortItem::Type::In, "IN" );
        break;
    case qan::FlowNode::Type::Tint:
        flowNode = insertNode<FlowNode>(nullptr);
        break;
    default: return nullptr;
    }
    if ( flowNode )
        flowNode->installBehaviour(std::make_unique<FlowNodeBehaviour>());
    return flowNode;
}

} // ::qan
