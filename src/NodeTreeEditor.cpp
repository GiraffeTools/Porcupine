#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>

#include "NodeTreeItem.hpp"
#include "NodeTreeEditor.hpp"

NodeTreeEditor::NodeTreeEditor(
        ) :
    QFrame(),
    m_layout(new QVBoxLayout())
{
    m_layout->setAlignment(Qt::AlignTop);

    setFrameShadow(QFrame::Raised);
    setFrameStyle(QFrame::StyledPanel);
    setLayout(m_layout);
}

void NodeTreeEditor::addNode(
        const Node* _node
        )
{
    m_nodes.append(_node);
    NodeTreeItem* item = new NodeTreeItem(_node, this);
    m_layout->addWidget(item);
    m_nodeList.append(item);
    item->setNumber(m_nodeList.length());
    connect(item, SIGNAL(moved(NodeTreeItem*)), this, SLOT(nodeMoved(NodeTreeItem*)));
}

void NodeTreeEditor::removeNode(
        const Node* _node
        )
{
    Q_UNUSED(_node)
//    for(int i = 0; i < rowCount(); ++i)
//    {
//        if(((NodeTreeItem*) item(i))->hasNode(_node))
//        {
//            removeRow(i);
//            break;
//        }
//    }
}

void NodeTreeEditor::nodeMoved(
        NodeTreeItem* _item
        )
{
    //remove the current _item
    m_nodeList.removeOne(_item);
    m_layout->removeWidget(_item);

    //The node needs to be placed after the last ancestor...
    NodeTreeItem* lastAncestor = 0;
    int begin = 0;
    for(int i = 0; i < m_nodeList.length(); ++i)
    {
        if(m_nodeList[i]->isAncestorOf(_item))
        {
            lastAncestor = m_nodeList[i];
        }
    }
    if(lastAncestor)
    {
        begin = m_nodeList.indexOf(lastAncestor) + 1;
    }

    NodeTreeItem* firstChild = 0;
    int end = m_nodeList.length();
    //...and before the first child
    for(int i = begin; i < end; ++i)
    {
        if(_item->isAncestorOf(m_nodeList[i]))
        {
            firstChild = m_nodeList[i];
            break;
        }
    }
    if(firstChild)
    {
        end = m_nodeList.indexOf(firstChild);
    }
    int index = end;

    if(begin != end) //if begin == end then it's pretty clear where our node should be
    {
        for(int i = begin; i < end; i++)
        {
            //if the item is placed above me, then insert here
            if(m_nodeList[i]->y() > _item->y())
            {
                index = i;
                break;
            }
        }
    }
    m_nodeList.insert(index, _item);
    m_layout->insertWidget(index, _item);

    updateNodeOrder();
}

void NodeTreeEditor::updateNodeOrder(
        )
{
    for(int i = 0; i < m_nodeList.length(); ++i)
    {
        m_nodeList[i]->setNumber(i + 1);
    }
}

//#include <iostream>
//void NodeTreeModel::dragEnterEvent(
//        QDropEvent* _event
//        )
//{
//    std::cerr << _event->mimeData()->text().toStdString() << " A\n";
//    _event->acceptProposedAction();
//}

//void NodeTreeModel::dropEvent(
//        QDropEvent* _event
//        )
//{
//    std::cerr << _event->mimeData()->text().toStdString() << " B\n";
//}

#include <iostream>
#include "Node.hpp"
void NodeTreeEditor::linkCreated(
        const Node* _from,
        const Node* _to
        )
{
    int toIndex = nodeIndexInList(_to);
    int fromIndex = nodeIndexInList(_from);
    if(fromIndex < toIndex)
    {
        //the list is already in the right order
        return;
    }
    QVector<const Node*> _descendants = m_nodeList[toIndex]->getDescendants();
    _descendants.prepend(_to);

    QVector<NodeTreeItem*> descendantItems = getNodeTreeItems(_descendants);
    QList<int> childIndices = nodeIndexInList(_descendants);

    int positioningIndex = std::max(fromIndex, childIndices.length() - 1);
    ///@todo find a less drastic solution
    for(int i = 0; i < childIndices.length(); ++i)
    {
        m_nodeList.removeOne(descendantItems[i]);
        m_layout->removeWidget(descendantItems[i]);

        m_nodeList.insert(positioningIndex, descendantItems[i]);
        m_layout->insertWidget(positioningIndex, descendantItems[i]);
    }
    updateNodeOrder();
}

NodeTreeItem* NodeTreeEditor::getNodeTreeItem(
        const Node* _node
        )
{
    return m_nodeList[nodeIndexInList(_node)];
}

QVector<NodeTreeItem*> NodeTreeEditor::getNodeTreeItems(
        QVector<const Node*> _nodes
        )
{
    QVector<NodeTreeItem*> list(_nodes.length());
    for(int i = 0; i < _nodes.length(); ++i)
    {
        list[i] = m_nodeList[nodeIndexInList(_nodes[i])];
    }
    return list;
}

int NodeTreeEditor::nodeIndexInList(
        const Node* _node
        )
{
    for(int i = 0; i < m_nodeList.length(); ++i)
    {
        if(m_nodeList[i]->getNode() == _node)
        {
            return i;
        }
    }
    // If we cannot find the Node
    return -1;
}

QList<int> NodeTreeEditor::nodeIndexInList(
        QVector<const Node*> _nodes
        )
{
    QList<int> indices;
    for(int i = 0; i < _nodes.length(); ++i)
    {
        indices.append(nodeIndexInList(_nodes[i]));
    }
    std::sort(indices.begin(), indices.end());
    return indices;
}

NodeTreeEditor::~NodeTreeEditor(
        )
{
    foreach (NodeTreeItem* item, m_nodeList)
    {
        delete item;
    }
}

