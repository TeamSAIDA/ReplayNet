#pragma once

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>

#include "Common.h"
#include "GameCommander.h"
#include "UXManager.h"

namespace MyBot
{

	class MyBotModule : public BWAPI::AIModule
	{
		/// ���� �����α׷�<br>
		/// @see GameCommander
		GameCommander   gameCommander;

		/// ����ڰ� �Է��� text �� parse �ؼ� ó���մϴ�
		void ParseTextCommand(const std::string & commandLine);

	public:

		/// ��Ⱑ ���۵� �� ��ȸ������ �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onStart();
		///  ��Ⱑ ����� �� ��ȸ������ �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onEnd(bool isWinner);
		/// ��� ���� �� �� �����Ӹ��� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onFrame();

		/// ����(�ǹ�/��������/��������)�� Create �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onUnitCreate(BWAPI::Unit unit);
		///  ����(�ǹ�/��������/��������)�� Destroy �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onUnitDestroy(BWAPI::Unit unit);

		/// ����(�ǹ�/��������/��������)�� Morph �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		/// Zerg ������ ������ �ǹ� �Ǽ��̳� ��������/�������� ���꿡�� ���� ��κ� Morph ���·� ����˴ϴ�
		void onUnitMorph(BWAPI::Unit unit);

		/// ����(�ǹ�/��������/��������)�� �Ҽ� �÷��̾ �ٲ� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�.<br>
		/// Gas Geyser�� � �÷��̾ Refinery �ǹ��� �Ǽ����� ��, Refinery �ǹ��� �ı��Ǿ��� ��, Protoss ���� Dark Archon �� Mind Control �� ���� �Ҽ� �÷��̾ �ٲ� �� �߻��մϴ�
		void onUnitRenegade(BWAPI::Unit unit);
		/// ����(�ǹ�/��������/��������)�� �ϴ� �� (�ǹ� �Ǽ�, ���׷��̵�, �������� �Ʒ� ��)�� ������ �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onUnitComplete(BWAPI::Unit unit);

		/// ����(�ǹ�/��������/��������)�� Discover �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�.<br>
		/// �Ʊ� ������ Create �Ǿ��� �� ��簡, ���� ������ Discover �Ǿ��� �� �߻��մϴ�
		void onUnitDiscover(BWAPI::Unit unit);
		/// ����(�ǹ�/��������/��������)�� Evade �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�.<br>
		/// ������ Destroy �� �� �߻��մϴ�
		void onUnitEvade(BWAPI::Unit unit);

		/// ����(�ǹ�/��������/��������)�� Show �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�.<br>
		/// �Ʊ� ������ Create �Ǿ��� �� ��簡, ���� ������ Discover �Ǿ��� �� �߻��մϴ�
		void onUnitShow(BWAPI::Unit unit);
		/// ����(�ǹ�/��������/��������)�� Hide �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�.<br>
		/// ���̴� ������ Hide �� �� �߻��մϴ�
		void onUnitHide(BWAPI::Unit unit);

		/// �ٹ̻��� �߻簡 �����Ǿ��� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onNukeDetect(BWAPI::Position target);

		/// �ٸ� �÷��̾ ����� ������ �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onPlayerLeft(BWAPI::Player player);
		/// ������ ������ �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onSaveGame(std::string gameName);

		/// �ؽ�Ʈ�� �Է� �� ���͸� �Ͽ� �ٸ� �÷��̾�鿡�� �ؽ�Ʈ�� �����Ϸ� �� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onSendText(std::string text);
		/// �ٸ� �÷��̾�κ��� �ؽ�Ʈ�� ���޹޾��� �� �߻��ϴ� �̺�Ʈ�� ó���մϴ�
		void onReceiveText(BWAPI::Player player, std::string text);
	};

}