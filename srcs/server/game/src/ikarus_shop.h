#ifndef __INCLUDE_NEW_OFFLINESHOP_HEADER__
#define __INCLUDE_NEW_OFFLINESHOP_HEADER__

#ifdef ENABLE_IKASHOP_RENEWAL
#define __USE_PID_AS_GUESTLIST__
#include <ranges>
#include "item.h"

//copyarray
template <class T, size_t size>
void CopyArray(T (&objDest)[size] , const T (&objSrc)[size]){
	if(size==0)
		return;
	memcpy(&objDest[0] , &objSrc[0], sizeof(T)*size);
}

template <class T>
void DeletePointersContainer(T& obj){
	typename T::iterator it = obj.begin();
	for(; it != obj.end(); it++)
		delete(*it);
}

namespace ikashop
{
	//patch 08-03-2020
	inline ikashop::ExpirationType GetItemExpiration(LPITEM item) {
		auto proto = item->GetProto();
		for (const auto limit : proto->aLimits) {
			if (limit.bType == LIMIT_REAL_TIME)
				return ikashop::ExpirationType::EXPIRE_REAL_TIME;
			else if (limit.bType == LIMIT_REAL_TIME_START_FIRST_USE && item->GetSocket(1) != 0)
				return ikashop::ExpirationType::EXPIRE_REAL_TIME_FIRST_USE;
		} return ikashop::ExpirationType::EXPIRE_NONE;
	}


	enum eOffshopChatPacket
	{
		CHAT_PACKET_CANNOT_CREATE_SHOP,
		CHAT_PACKET_CANNOT_CHANGE_NAME,
		CHAT_PACKET_CANNOT_FORCE_CLOSE,
		CHAT_PACKET_CANNOT_OPEN_SHOP,
		CHAT_PACKET_CANNOT_OPEN_SHOP_OWNER,

		CHAT_PACKET_CANNOT_ADD_ITEM,
		CHAT_PACKET_CANNOT_BUY_ITEM, //tofix wrong chat packet
		CHAT_PACKET_CANNOT_REMOVE_ITEM,
		CHAT_PACKET_CANNOT_EDIT_ITEM,
		CHAT_PACKET_CANNOT_REMOVE_LAST_ITEM,

		CHAT_PACKET_CANNOT_FILTER,
		CHAT_PACKET_CANNOT_SEARCH_YET,
		CHAT_PACKET_OFFER_CREATE,

		CHAT_PACKET_CANNOT_CREATE_OFFER,
		CHAT_PACKET_CANNOT_ACCEPT_OFFER,

		CHAT_PACKET_CANNOT_OPEN_SAFEBOX,
		CHAT_PACKET_CANNOT_SAFEBOX_GET_ITEM,
		CHAT_PACKET_CANNOT_SAFEBOX_GET_VALUTES,
		CHAT_PACKET_CANNOT_SAFEBOX_CLOSE,


		CHAT_PACKET_RECV_ITEM_SAFEBOX,

		//AUCTION
		CHAT_PACKET_AUCTION_CANNOT_SEND_LIST,
		CHAT_PACKET_AUCTION_CANNOT_OPEN_AUCTION,
		CHAT_PACKET_AUCTION_CANNOT_CREATE_AUCTION,
		CHAT_PACKET_AUCTION_CANNOT_ADD_OFFER,

		//GENERAL
		CHAT_PACKET_CANNOT_DO_NOW,

	};


	class CShopItem
	{
		using ItemID = DWORD;
		using OwnerID = DWORD;

	public:
		TItemTable* GetTable() const;
		LPITEM CreateItem() const;

		const TPriceInfo& GetPrice() const;
		void SetPrice(const TPriceInfo& price);
		
		const TShopItem& GetInfo() const;
		void SetInfo(const TShopItem& info);

		ItemID GetID() const;
		bool CanBuy(LPCHARACTER ch);

		void SetCell(int pos);

	protected:
		TShopItem m_info{};

	};

	template<int WIDTH, int HEIGHT>
	class CShopGrid
	{
		static constexpr auto CELL_COUNT = WIDTH*HEIGHT;

	public:
		void Clear()
		{
			m_cells = {};
		}

		void RegisterItem(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			for(int i=0; i < size; i++)
				_SetCellState(_ComposeCell(col, row+i), true);
		}

		void UnregisterItem(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			for (int i = 0; i < size; i++)
				_SetCellState(_ComposeCell(col, row + i), false);
		}

		bool CheckSpace(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			for(int nrow = row; nrow < row + size; nrow++)
				if(_GetCellState(_ComposeCell(col, nrow)))
					return false;
			return true;
		}

		std::optional<int> FindSpace(int size)
		{
			for(int cell = 0; cell < CELL_COUNT; cell++)
				if(CheckSpace(cell, size))
					return cell;
			return std::nullopt;
		}

		constexpr int GetCellCount(){
			return WIDTH*HEIGHT;
		}

	private:
		std::pair<int,int> _DecomposeCell(int cell){ return { cell % WIDTH, cell / WIDTH }; }
		int _ComposeCell(int col, int row){ return row * WIDTH + col; }
		void _SetCellState(int cell, bool state){ if(cell >= 0 && cell < CELL_COUNT) m_cells[cell] = state; }
		bool _GetCellState(int cell) { return cell >= 0 && cell < CELL_COUNT ? m_cells[cell] : true; }

	private:
		std::array<bool, WIDTH*HEIGHT> m_cells{};
	};

	class ShopEntity;

	class CShop
	{
	public:
		static constexpr auto GRID_WIDTH = 15;
		static constexpr auto GRID_HEIGHT = 10;
#ifdef EXTEND_IKASHOP_ULTIMATE
		static constexpr auto PAGE_COUNT = 3;
		static constexpr auto GRID_SIZE = GRID_WIDTH*GRID_HEIGHT;
#endif

	public:
		using ITEM_HANDLE = std::shared_ptr<CShopItem>;
		using OFFER_HANDLE = std::shared_ptr<TOfferInfo>;

		using SHOP_ITEM_MAP = std::map<DWORD, ITEM_HANDLE>;
		using OFFER_MAP = std::map<DWORD, OFFER_HANDLE>;

		using VECGUEST = std::set<DWORD>;
		using SHOPNAME = std::string;
#ifdef ENABLE_IKASHOP_ENTITIES
		using ENTITY_HANDLE = std::shared_ptr<ShopEntity>;
#endif
	public:
		// get const 
		const SHOP_ITEM_MAP& GetItems() const;
		const OFFER_MAP& GetOffers() const;
		const VECGUEST& GetGuests() const;

		// duration
		void SetDuration(DWORD duration);
		void DecreaseDuration();
		DWORD GetDuration() const;

		// owner pid
		void SetOwnerPID(DWORD ownerid);
		DWORD GetOwnerPID() const;

		// guests
		bool AddGuest(LPCHARACTER ch);
		bool RemoveGuest(LPCHARACTER ch);
		void KickAllGuests();

		// items
		bool AddItem(const TShopItem& rItem);
		bool RemoveItem(DWORD itemid);
		bool ModifyItemPrice(DWORD itemid, const TPriceInfo& price);
		bool BuyItem(DWORD itemid);
		ITEM_HANDLE GetItem(DWORD itemid);
			 
		// offers
		bool AddOffer(OFFER_HANDLE offer);
		bool AcceptOffer(OFFER_HANDLE offer);
		void NotifyOffers(LPCHARACTER ch);
		void NotifyAcceptedOffers(LPCHARACTER ch);
		void RemoveOffer(DWORD offerid);

		// helpers
		LPCHARACTER FindOwnerCharacter();
		void Clear();
		void RefreshToOwner();
		void SetName(const SHOPNAME& name);
		const char* GetName() const;

		// grid checks
		bool ReserveSpace(int cell, int size);

		int GetNewOffersCounter() const;

#ifdef EXTEND_IKASHOP_ULTIMATE
		void MoveItem(DWORD itemid, int destCell);
		void ChangeDuration(DWORD duration);
		
		void SetDecoration(int decoration);
		void SetDecorationDuration(int decoration_time);
		int GetDecoration() const;
		int GetDecorationTime() const;
		
		void SetLockIndex(int lockIndex);
		int GetLockIndex() const;
		void ChangeLockIndex(int lockIndex);
#endif

		void SetEntity(ENTITY_HANDLE handle);
		ENTITY_HANDLE GetEntity() const;

#ifdef ENABLE_IKASHOP_ENTITIES
		void SetSpawn(const TShopSpawn& spawn);
		const TShopSpawn& GetSpawn() const;
#endif

	private:
		void _RefreshViews(LPCHARACTER ch = nullptr);
		void _SendNotifyOffer(LPCHARACTER ch, const TOfferInfo& offer);
#ifdef EXTEND_IKASHOP_ULTIMATE
		void _RefreshGrid();
#endif

	private:
		ENTITY_HANDLE m_entity;
		SHOP_ITEM_MAP m_items{};
		OFFER_MAP m_offers{};
		VECGUEST m_guests{};
		DWORD m_pid{};
		DWORD m_duration{};
		SHOPNAME m_name{};
#ifdef EXTEND_IKASHOP_ULTIMATE
		CShopGrid<GRID_WIDTH, GRID_HEIGHT> m_grid[PAGE_COUNT];
#else
		CShopGrid<GRID_WIDTH, GRID_HEIGHT> m_grid;
#endif
#ifdef ENABLE_IKASHOP_ENTITIES
		TShopSpawn m_spawn;
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		int m_decoration = 0;
		int m_decorationTime = 0;
		int m_lockIndex = 0;
#endif
	};


	class CShopSafebox
	{
	public:
		static constexpr auto GRID_WIDTH = 9;
		static constexpr auto GRID_HEIGHT = 11;

	public:
		using ITEM_HANDLE = std::shared_ptr<CShopItem>;
		using ITEM_MAP = std::map<DWORD, ITEM_HANDLE>;

	public:
		CShopSafebox() = default;
		CShopSafebox(const CShopSafebox&) = default;
		CShopSafebox(LPCHARACTER chOwner);

		void SetOwner(LPCHARACTER ch);
		void SetValuteAmount(TValutesInfo val);
			 
		bool AddItem(const TShopPlayerItem& item);
		bool RemoveItem(DWORD itemid);
			 
		void AddValute(TValutesInfo val);
		bool RemoveValute(TValutesInfo val);

		const ITEM_MAP& GetItems() const;
		const TValutesInfo& GetValutes() const;

		ITEM_HANDLE GetItem(DWORD itemid) const;
		LPCHARACTER GetOwner() const;

		bool RefreshToOwner(LPCHARACTER ch = nullptr);

	private:
		int _RegisterGridPosition(ITEM_HANDLE item);
		void _UnregisterGridPosition(ITEM_HANDLE item);


	private:
		ITEM_MAP m_items{};
		LPCHARACTER m_owner{};
		TValutesInfo m_valutes{};
		std::vector<CShopGrid<GRID_WIDTH, GRID_HEIGHT>> m_grids;
	};


	class CAuction
	{
	public:
		using AUCTION_OFFER_HANDLE = std::shared_ptr<TAuctionOfferInfo>;
		using AUCTION_OFFERVEC = std::vector<AUCTION_OFFER_HANDLE>;
		using GUESTVEC = std::vector<DWORD>;
#ifdef ENABLE_IKASHOP_ENTITIES
	#ifdef EXTEND_IKASHOP_ULTIMATE
			using ENTITY_HANDLE = std::shared_ptr<ShopEntity>;
	#endif
#endif

	public:
		void SetInfo(const TAuctionInfo& auction);
		void SetOffers(const std::vector<TAuctionOfferInfo>& vec);

		bool AddOffer(const TAuctionOfferInfo& offer, bool check = true);
		bool AddGuest(LPCHARACTER ch);
		bool RemoveGuest(LPCHARACTER ch);
		void DecreaseDuration();

		//adding 1 minute time when new offer is done in the last minute
		void IncreaseDuration();
		
		const GUESTVEC& GetGuests() const;
		const DWORD	GetBestBuyer() const;
		const TAuctionInfo& GetInfo() const;
		const AUCTION_OFFERVEC& GetOffers() const;
		AUCTION_OFFER_HANDLE GetBestOffer() const;

		void RefreshToOwner(LPCHARACTER ch = nullptr);

		int GetNewOffersCounter() const;

#ifdef ENABLE_IKASHOP_ENTITIES
	#ifdef EXTEND_IKASHOP_ULTIMATE
		void SetEntity(ENTITY_HANDLE entity);
		ENTITY_HANDLE GetEntity();
		const TShopSpawn& GetSpawn() const;
	#endif
#endif

	private:
		void RefreshGuestView();
		bool CheckBestOffer();

	private:
		TAuctionInfo m_info{};
		AUCTION_OFFERVEC m_offers{};
		AUCTION_OFFER_HANDLE m_bestOffer;
		GUESTVEC m_guests{};
#ifdef ENABLE_IKASHOP_ENTITIES
	#ifdef EXTEND_IKASHOP_ULTIMATE
		ENTITY_HANDLE m_entity;
	#endif
#endif

	};

#ifdef ENABLE_IKASHOP_ENTITIES
	class ShopEntity : public CEntity
	{
	private:
		static DWORD AllocID() {
			static auto id = 0UL;
			return ++id;
		}

	public:
		using SHOP_HANDLE = std::shared_ptr<CShop>;
#ifdef EXTEND_IKASHOP_ULTIMATE
		using AUCTION_HANDLE = std::shared_ptr<CAuction>;
#endif

	public:
		ShopEntity();

		//overriding virtual CEntity methods
		void EncodeInsertPacket(LPENTITY entity);
		void EncodeRemovePacket(LPENTITY entity);
		
		const char* GetShopName() const;
		void SetShopName(const char* name);

		int GetShopType() const;
		void SetShopType(int iType);
		void SetShop(SHOP_HANDLE shop);
#ifdef EXTEND_IKASHOP_ULTIMATE
		void SetAuction(AUCTION_HANDLE auction);
		AUCTION_HANDLE GetAuction();
#endif

		SHOP_HANDLE GetShop() {return m_shop;}
		void Destroy() { CEntity::Destroy(); }

		DWORD GetVID() const { return m_vid; }

	private:
		std::string m_name{};
		int m_type{};
		SHOP_HANDLE m_shop{};
#ifdef EXTEND_IKASHOP_ULTIMATE
		AUCTION_HANDLE m_auction{};
#endif
		DWORD m_vid = AllocID();
	};
#endif
}

#endif

#endif //__include
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
