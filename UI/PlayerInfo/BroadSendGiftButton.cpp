#include "BroadSendGiftButton.h"
#include "Layer.h"

BroadSendGiftButton::BroadSendGiftButton(CCSize cellSize):
	m_ptrNormal(NULL),
	m_ptrSelected(NULL)
{
	m_ptrNormal = Layer::create();
	m_ptrNormal->setContentSize(cellSize);

	m_ptrSelected = Layer::create();
	m_ptrSelected ->setContentSize(cellSize);
}

void BroadSendGiftButton::init(CCLayer* parentLayer)
{
	initWithNormalSprite(m_ptrNormal, m_ptrSelected, NULL, parentLayer, NULL);
}
