#ifndef __INCLUDE_HEADER_OFFLINESHOP_CACHE__
#define __INCLUDE_HEADER_OFFLINESHOP_CACHE__

#ifdef ENABLE_IKASHOP_RENEWAL
#include <numeric>
namespace ikashop
{
	struct SQueryInfoCreateShop;

	class CShopCache
	{
	public:
		struct TShopCacheItemInfo : TShopItem {
			bool lock = false;
		};

		using ITEM_CACHE_HANDLE = std::shared_ptr<TShopCacheItemInfo>;
		using SHOPCACHE_MAP = std::map<DWORD, ITEM_CACHE_HANDLE>;
		struct TShopCacheInfo 
		{
			DWORD duration;
			SHOPCACHE_MAP itemsmap;
			std::string name;
#ifdef ENABLE_IKASHOP_ENTITIES
			TShopSpawn spawn;
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
			int decoration;
			int decoration_time;
			int lock_index;
#endif
		};
		using SHOP_HANDLE = std::shared_ptr<TShopCacheInfo>;

	public:
		CShopCache();
		~CShopCache();

		SHOP_HANDLE	Get(DWORD dwOwnerID) const;
		bool AddItem(const TShopCacheItemInfo& rItem);
		bool RemoveItem(DWORD dwOwnerID, DWORD dwItemID);
		void DeleteItem(DWORD owner, DWORD itemid);
		bool SellItem(DWORD dwOwnerID, DWORD dwItemID, bool updateQuery = true);
		bool LockSellItem(DWORD dwOwnerID, DWORD dwItemID, long long TotalPriceSeen); //patch seen price check
		bool UnlockSellItem(DWORD dwOwnerID, DWORD dwItemID);//topatch
		bool EditItem(DWORD dwOwnerID, DWORD dwItemID, const TPriceInfo& rItemPrice);			 
		bool CloseShop(DWORD dwOwnerID);
#ifdef ENABLE_IKASHOP_ENTITIES
		bool CreateShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName, const TShopSpawn& spawn);
#else
		bool CreateShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName);
#endif
		bool ChangeShopName(DWORD dwOwnerID, const char* szName);
			
		// on boot
		bool PutItem(DWORD dwOwnerID, DWORD dwItemID, const TShopCacheItemInfo& rItem);
		bool PutShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName
#ifdef ENABLE_IKASHOP_ENTITIES
			, const TShopSpawn& spawn
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
			, int decoration, int decoration_time, int lock_index
#endif
		);
			 
		DWORD GetCount() const {return m_shopsMap.size();}
		void EncodeCache(CPeer* peer) const;
		DWORD GetItemCount() const;
			 
		void ShopDurationProcess();
		void UpdateDurationQuery(DWORD dwOwnerID, const TShopCacheInfo& rShop );

		ITEM_CACHE_HANDLE GetItem(DWORD owner, DWORD item);

#ifdef ENABLE_IKASHOP_ENTITIES
#ifdef EXTEND_IKASHOP_PRO
		bool MoveShopEntity(DWORD owner, const TShopSpawn& spawn);
#endif
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		bool ChangeDecoration(DWORD owner, int index);
		void UpdateDecorationTime(DWORD owner, int time);
		bool MoveItem(DWORD owner, DWORD itemid, int destCell);
#endif

	private:
		std::map<DWORD, SHOP_HANDLE> m_shopsMap;
	};


	class CSafeboxCache
	{
	public:
		struct TSafeboxCacheInfo {
			TValutesInfo valutes;
			std::map<DWORD, TShopPlayerItem> itemsmap;
			std::map<DWORD, TShopPlayerItem> lockedmap;
		};
		
		using SAFEBOX_HANDLE = std::shared_ptr<TSafeboxCacheInfo>;
		using CACHEMAP = std::map<DWORD, SAFEBOX_HANDLE>;

	public:
		CSafeboxCache();
		~CSafeboxCache();

		SAFEBOX_HANDLE Get(DWORD dwOwnerID) const;

		bool PutSafebox(DWORD dwOwnerID, const TSafeboxCacheInfo& rSafebox);
		bool PutItem(DWORD dwOwnerID, DWORD dwItem, const TShopPlayerItem& item);

		bool DeleteItem(DWORD dwOwner, DWORD dwItemID);
		bool LockItem(DWORD dwOwner, DWORD dwItemID);
		bool RemoveLockedItem(DWORD dwOwner, DWORD dwItemID);
		bool UnlockItem(DWORD dwOwner, DWORD dwItemID);
		bool AddItem(DWORD dwOwnerID, DWORD dwItemID, const TShopPlayerItem& item);

		bool AddValutes(DWORD dwOwnerID, const TValutesInfo& val);
		bool RemoveValutes(DWORD dwOwnerID, const TValutesInfo& val);
		void AddValutesAsQuery(DWORD dwOwnerID, const TValutesInfo& val);

		SAFEBOX_HANDLE CreateSafebox(DWORD dwOwnerID);
		DWORD GetCount() const {return m_safeboxMap.size();}
		DWORD GetItemCount() const;

		SAFEBOX_HANDLE LoadSafebox(DWORD dwPID);

		//patch 08-03-2020
		void ItemExpirationProcess();

	private:
		CACHEMAP m_safeboxMap;

	};


	class COfferCache
	{
	public:
		using OFFER_HANDLE = std::shared_ptr<TOfferInfo>;
		using CACHEMAP = std::map<DWORD, OFFER_HANDLE>;
		using FINDOFFERBYSHOP = std::map<DWORD, std::vector<OFFER_HANDLE>>;

	public:
		COfferCache();
		~COfferCache();

		bool Puts(const TOfferInfo& rOffer);
		bool AddOffer(const TOfferInfo& rOffer);
		bool NotifiedOffer(DWORD dwOfferID);
		bool AcceptOffer(DWORD dwOfferID);
		bool CancelOffer(DWORD dwOfferID,DWORD dwOwnerID);
		bool RemoveOffersByShopOwner(DWORD dwShopOwnerID);

		OFFER_HANDLE Get(DWORD dwOfferID) const;
		std::vector<OFFER_HANDLE> GetOffersByShopOwner(DWORD dwOwnerID) const;
		std::vector<OFFER_HANDLE> GetOffersByItemID(DWORD dwOwnerID, DWORD dwItemID) const;

		DWORD GetCount() {return m_mapOffersByBuyer.size();}
		void EncodeCache(CPeer* peer) const;

	private:
		CACHEMAP m_mapOffersByBuyer;
		FINDOFFERBYSHOP m_findOffersByShop;
	};

	class CAuctionCache 
	{
	public:
		using AUCTION_HANDLE = std::shared_ptr<TAuctionInfo>;
		using AUCTION_OFFER_HANDLE = std::shared_ptr<TAuctionOfferInfo>;
		using AUCTION_OFFER_VEC = std::vector<AUCTION_OFFER_HANDLE>;
		using AUCTION_MAP = std::map<DWORD, std::shared_ptr<TAuctionInfo>>;
		using AUCTION_OFFER_MAP = std::map<DWORD, AUCTION_OFFER_VEC>;

	public:
		CAuctionCache();
		~CAuctionCache();

		bool PutsAuction(const TAuctionInfo& auction);
		bool PutsAuctionOffer(const TAuctionOfferInfo& auctionOffer);

		bool AddAuction(const TAuctionInfo& auction);
		bool AddOffer(const TAuctionOfferInfo& auctionOffer, bool quering = true);
		bool ExpiredAuction(DWORD dwOwnerID);
		bool AuctionDurationProcess();

		AUCTION_OFFER_HANDLE GetBestBuyerOffer(DWORD dwOwnerID);

		AUCTION_HANDLE Get(DWORD dwOwnerID);
		AUCTION_OFFER_VEC GetOffers(DWORD dwOwnerID);

		DWORD GetCount();
		DWORD GetOffersCount();

		void EncodeCache(CPeer* peer);

	private:
		void _UpdateDuration(const TAuctionInfo& auction);

	private:
		AUCTION_MAP m_mapAuction;
		AUCTION_OFFER_MAP m_mapAuctionOffer;
	};

#ifdef EXTEND_IKASHOP_ULTIMATE
	class CAveragePriceCache 
	{
		using COOLDOWN_VNUM_MAP = std::map<DWORD, int64_t>;
		using COOLDOWN_ACCOUNT_MAP = std::map<DWORD, COOLDOWN_VNUM_MAP>;
		using AVERAGE_PRICE_VECTOR = std::vector<TSaleHistory>;

	public:
		bool InitializeTable();

		bool RegisterSelling(const TSaleHistory& data);
		void Encode(CPeer* peer);
		DWORD GetCount() { return m_priceAverageVector.size(); }

	private:
		COOLDOWN_ACCOUNT_MAP m_cooldownMap;
		AVERAGE_PRICE_VECTOR m_priceAverageVector;
	};
#endif


	//QUERY INFO
	struct SQueryInfoCreateShop {
		DWORD owner;
		DWORD duration;
		std::string name;
#ifdef ENABLE_IKASHOP_ENTITIES
		TShopSpawn spawn;
#endif
	};
	using SQueryInfoAddItem = CShopCache::TShopCacheItemInfo;	
	using SQueryInfoSafeboxAddItem = TShopPlayerItem;
	using SQueryInfoOfferAdd = TOfferInfo;
}

	



#endif
#endif //__INCLUDE_HEADER_OFFLINESHOP_CACHE__
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
