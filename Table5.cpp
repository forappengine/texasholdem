#include "Table5.h"
#include "Player5.h"
#include "LogTag.h"
#include "LayoutUtil.h"

#define MODULE_LOG_TAG "Table5.cpp"

//node is joined player
Player* Table5::createAndAddPlayer(GameScreen* gameScreen,const JSONNode& node)
{
	int index = node["index"].as_int();
	int seat = (capacity + index - myPosition) % capacity;//move my position to the middle of the bottom
	Player* player = Player5::create(this,seat,node);
	addChild(player,Z_PLAYER);
	players[index] = player;
	layoutPlayer(index);
	hideSitDownBtn(index);
	return player;
}

void Table5::layoutSitDownBtn(int index)
{
	int seat = (capacity + index - myPosition) % capacity;
	switch(seat)
	{
	case 0:
		LayoutUtil::layoutParentCenter(m_prtBtnSitDown[index],0,-162);
		break;
	case 1:
		LayoutUtil::layoutParentCenter(m_prtBtnSitDown[index],-370,-82);
		break;
	case 2:
		LayoutUtil::layoutParentCenter(m_prtBtnSitDown[index],-175,195);
		break;
	case 3:
	LayoutUtil::layoutParentCenter(m_prtBtnSitDown[index],175,195);
		break;
	case 4:
		LayoutUtil::layoutParentCenter(m_prtBtnSitDown[index],370,-82);
		break;
	}
}

void Table5::onEnter()
{
	TableLayer::onEnter();
	capacity = 5;
	players = new Player * [capacity];
	m_prtBtnSitDown = new CCMenuItemSprite * [capacity];
	for(int i = 0; i < capacity; i++)
	{
		players[i] = NULL;
		m_prtBtnSitDown[i] = NULL;
	}
}

void Table5::layoutPlayer( int index )
{
	Player* player = players[index];
	if (player == NULL)
	{
		return;
	}
	player->m_seatPosition = (capacity + player->m_index - myPosition) % capacity;
	switch(player->m_seatPosition)
	{
	case 0:
		LayoutUtil::layoutParentCenter(player,0,-155);
		break;
	case 1:
		LayoutUtil::layoutParentCenter(player,-370,-75);
		break;
	case 2:
		LayoutUtil::layoutParentCenter(player,-175,190);
		break;
	case 3:
		LayoutUtil::layoutParentCenter(player,175,190);
		break;
	case 4:
		LayoutUtil::layoutParentCenter(player,370,-75);
		break;
	}
}
