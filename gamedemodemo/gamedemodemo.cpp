/*
gamedemodemo.cpp - sven.pohl@zen-systems.de

compiled with cleos v2.1.0
*/


#include <eosio/eosio.hpp>
#include "base58.hpp" 

#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp> // tapos...

#include "gamedemodemo.hpp"     


/*
gamedemodemo. (11.okt.2022 - Beginn)

11.okt.2022 - Beginn

todo:
*/

using namespace eosio;
using namespace std;


 
const std::string   version   = "V1.0";
 

CONTRACT gamedemodemo : public eosio::contract {

  public:
      using contract::contract;
      
      // ---
      // Constants
      //
      const uint32_t     hashwert    = 23434;      

                
      
      //Scope: collection_name
      TABLE schemas_s {
        name            schema_name;
        vector <FORMAT> format;

        uint64_t primary_key() const { return schema_name.value; }
      };

      typedef multi_index <name("schemas"), schemas_s> schemas_t;
    
  
      //Scope: owner
      TABLE assets_s {
        uint64_t         asset_id;
        name             collection_name;
        name             schema_name;
        int32_t          template_id;
        name             ram_payer;
        vector <asset>   backed_tokens;
        vector <uint8_t> immutable_serialized_data;
        vector <uint8_t> mutable_serialized_data;

        uint64_t primary_key() const { return asset_id; };
      };

      typedef multi_index <name("assets"), assets_s> assets_t;
      
      
      
      
      struct param_struct
      {
      std::string param;    
      };



      struct transfer_args
      {
      name from;
      name to;
      asset quantity;
      std::string memo;
      };    


      struct transfer_args_nft
      {
      name from;
      name to;
      vector <uint64_t> sender_asset_ids;
      std::string memo;
      };    
          
          
      assets_t  get_assets(name acc) 
      {
      return assets_t( name("atomicassets")  , acc.value); 
      }

    
      schemas_t  get_schemas(name collection_name) 
      {
      return schemas_t(  name("atomicassets") , collection_name.value);         
      }
           
      
      
      ATTRIBUTE_MAP deserialize(const std::vector <uint8_t> &data, const std::vector <FORMAT> &format_lines) {
      ATTRIBUTE_MAP attr_map = {};

        auto itr = data.begin();
        while (itr != data.end()) {
            uint64_t identifier = unsignedFromVarintBytes(itr);
            FORMAT format = format_lines.at(identifier - RESERVED);
            
            print(" T:(",format.type, "-" ,format.name, ") ");
            attr_map[format.name] = deserialize_attribute(format.type, itr);
        }

      return attr_map;
      }
          
         
    
 
      // ---
      // struct global
      //
      TABLE global
      {
      uint64_t id; 
      int      usdelay;

      auto primary_key() const { return id; }
      EOSLIB_SERIALIZE(global, (id)(usdelay) )  
      }; // struct global      
      

      typedef eosio::multi_index< name("global"), global> globals;


 


      TABLE stake_s 
      {
      uint64_t asset_id; 
      name     user;
      int      status;      
      uint64_t ts; 
      uint64_t tsu; 
            
      uint64_t primary_key() const { return asset_id; }
      uint64_t by_user()    const { return user.value; } 
      };
      
      typedef eosio::multi_index <name("stake"), stake_s ,    
              indexed_by <name("byuser"), const_mem_fun <stake_s, uint64_t, &stake_s::by_user>>   > stake_t;
      
 
  
 
      
      // ---
      // Status - minimal state
      [[eosio::action]]  
      void status() 
      {       
      print(" VERSION gdemo: ",version," ");       
      } // status() 
    

  
    
      inline void splitMemo(std::vector<std::string> &results, std::string memo)
            {
            auto end = memo.cend();
            auto start = memo.cbegin();

            for (auto it = memo.cbegin(); it != end; ++it)
                {
                if (*it == ';')
                   {
                   results.emplace_back(start, it);
                   start = it + 1;
                   }
                }
            if (start != end)
            results.emplace_back(start, end);
            } // splitMemo





      // ---
      // cmessage  
      //
      [[eosio::action]]
      inline void cmessage(name to, std::string memo) 
           {    
           require_recipient(to);
           // require_recipient( get_self() );           
           } // cmessage




      // ---
      // send_token - sending token.
      //  
      inline void send_token(name receiver, uint64_t amount, std::string memo) 
      {             
      asset quant = asset(amount, symbol("EOS", 4)  );            
      action(
            permission_level{_self, name("active")},
            name("eosio.token"), name("transfer"),
            std::make_tuple(_self, receiver, quant, memo )
            ).send();
      } // void send_token()

  

      // ---
      // handle_transfer - NFT handler.
      //
      [[eosio::action]]  
      void hdlnft()
      {
      auto data = unpack_action_data<transfer_args_nft>();
    
 
      if ( data.from == get_self()) 
         {
         print(" DO NOTHING ");
         return;
         } 
         
      if(data.to != _self) 
         {
         print(" DO NOTHING_ ");
         return;
         }    
      
      name code = get_first_receiver();
      print("code: ",code);  
      print(" from: ", data.from );
      // print(" quantity: ", data.quantity.amount );
      // print(" symbol: ", data.quantity.symbol ); 
      print(" memo: ", data.memo );


      auto size = data.sender_asset_ids.size();
      print(" size: ", size , " ");
      
      check( size == 1 , " Please stake only one NFT! " ); // break
      
      uint64_t assetid = data.sender_asset_ids[0];

      print(" assetid: ", assetid, " " );
         
      // Get assets of the owneraccount 
      assets_t owner_assets = get_assets( data.to );
   
      auto asset_itr = owner_assets.require_find( assetid,   "No asset with this id does exists in this account");

           
      print("FOUND T: ", asset_itr->template_id , " "   ,   asset_itr->schema_name , " "   ,  asset_itr->collection_name , " "    );    
     
          
      if (
         (asset_itr->collection_name == name("xusoxusoxuso")  || asset_itr->collection_name == name("xusogamedemo") ) &&         
         // asset_itr->schema_name     == name("monsters") &&
         data.memo == "stake"
         )
         {
         print("  ASSET FOUND!");
        
         stake_t mystake(_self, _self.value);
          
         auto now = current_time_point().sec_since_epoch();
                                     
         mystake.emplace(_self, [&](auto&  tupel) 
                {                
                tupel.asset_id      = assetid;                
                tupel.user          = data.from;                            
                tupel.status        = 1; // start with "just staking"                
                tupel.ts            = now;
                tupel.tsu           = 0;                                                                                             
                }); 
           
         } else
             {
             check( 1 == 2 , " INVALID ASSET! " ); // break
             }
          
     
      print(" FIN: "  );
      } // hdlnft()
      
      
      
 

      // ---
      // handle_transfer - xxx.
      //
      [[eosio::action]]  
      void hdltransfer()
      {
      // other token (fungible tokens)
      } // hdltransfer
    
    
    

      // ---
      // claim - xxx
      //
      [[eosio::action]]  
      void claim( name user, uint64_t asset_id )
      {
      print(" claim-1b ");
      require_auth( user );
      
      stake_t mystake(_self, _self.value);
      
      auto iterator = mystake.find( asset_id ); 
      
      
      if ( iterator != mystake.end() &&
           iterator->user == user &&
           iterator->status == 1
         )
         {
         print(" FOUND ");       

         auto now = current_time_point().sec_since_epoch();
         auto delay = now - iterator->ts;
         
         uint64_t hours = delay / (60*60);      
         uint64_t reward_per_hour = 1000;
         uint64_t reward = hours * reward_per_hour;
         
             
         print(" delay: ",delay," " );
         print(" hours: ",hours," " );
         print(" reward: ",reward," " );
            
            
         if ( delay > (60*60) )
            { 
            // Issue SUN token
            asset quant = asset(reward, symbol("SUN", 4)  );   
            
            action(
                  permission_level{_self, name("active")},
                  name("tokentoken11"), name("issue"),
                  std::make_tuple( _self,  quant,  std::string("Gamedemodemo Stake Rewards issue") )                                   
                  ).send();



            // Transfer             
            action(
                  permission_level{_self, name("active")},
                  name("tokentoken11"), name("transfer"),
                  std::make_tuple(_self, user, quant, std::string("Gamedemodemo Stake Rewards")  )
                  ).send();
            
 
            // Reset timer
            mystake.modify(iterator, _self, [&](auto& tupel) 
                   {                                                                            
                   tupel.ts     = now;              
                   });  
                
            } // if (delay > (60*60)
            else
                {
                check( 1 == 2 , " AT LEAST ONE HOUR! " ); // break
                }

         
         
         } else
           {
           print(" NOT FOUND ");
           check( 1 == 2 , " NOT FOUND " );
           }     

 
      print(" FIN ");
      } // claim
 
 

      // ---
      // unstake 
      //
      [[eosio::action]]  
      void unstake( name user, uint64_t asset_id )
      {
      print(" unstake1 ");
      require_auth( user );
    
    
      int id = 0;
      globals myglobals( _self , _self.value );         
      auto iterator_globals = myglobals.find(id);            
    
       
      print(" iterator_globals: ",iterator_globals->usdelay   ," " );
                                 
      stake_t mystake(_self, _self.value);
      
      auto iterator = mystake.find( asset_id ); 
      print( " status:"   ,iterator->status  ," " );
         
         
      // Init unstake process
      if ( iterator != mystake.end() &&
           iterator->user == user &&
           iterator->status == 1
         )
         {
         print(" FOUND s1 ");  
         
         auto now = current_time_point().sec_since_epoch();
                       
         // Update status
         mystake.modify(iterator, _self, [&](auto& tupel) 
                 {                                                                            
                 tupel.status     = 2;    
                 tupel.ts         = 0;          
                 tupel.tsu        = now;          
                 });  
       
    
         print(" exit ");
         return;         
         } else
           {
           print(" NOT FOUND Status 1 ");
           }     
           
           
 
           
      // Check if unstaking-process is initiated
      if ( iterator != mystake.end() &&
           iterator->user == user &&
           iterator->status == 2
         )
         {
         print(" FOUND s2 ");  
        
         auto now = current_time_point().sec_since_epoch();
              
         auto delay = now - iterator->tsu;
         print(" delay: ",delay," ");  
     
         // Send asset back, remove DB-Entry
         if ( delay >= iterator_globals->usdelay )
            {
            print(" UNSTAKING ");
                 
  
            // send asset
            vector <uint64_t> sender_asset_id;
   
            sender_asset_id.push_back( asset_id );
            std::string memo = "unstake NFT";
         
            action(
                  permission_level{_self, name("active")},
                  name("atomicassets"), name("transfer"),
                  std::make_tuple( _self, iterator->user, sender_asset_id, memo )
                  ).send();
            
                        
            // delete table entry           
            mystake.erase( iterator );                        
            } else
              {         
              print("Needs to wait ",iterator_globals->usdelay," seconds ");      
              check( 1 == 2 , " UNSTAKING TO EARLY " ); // break
              }
 
         } else
           {
           print(" NOT FOUND Status 2 ");
           }                
      
 
      print(" FIN ");
      } // claim
 


 
      
      // ---
      // admin - superuser functions.
      //
      [[eosio::action]]  
      void admin( std::string param ) 
      {      
      print(" ADMIN_1 ");     
      require_auth(get_self());
                     
      std::vector<std::string> results;
      splitMemo(results,   param );
            
      
      print(" first:...[", results[0] ,"] ");
      
             
      if (results[0] == "init")
         {
         print(" Init... ");
         
         // ---
         // Globals
         //
         int id = 0;
         globals myglobals( _self , _self.value );         
         auto iterator = myglobals.find(id);
    
         if ( iterator != myglobals.end() )
            {
            print(" globals does exist ");
            } else
              {
              print(" globals will be created.... ");
              myglobals.emplace(_self, [&](auto& global) 
                 {
                 global.id = id;
                 global.usdelay        = 60*3; // 3 minutes
                 });
             } // else
            
         } // if (data.param == "init")
             
      } // admin()
      
      
 
      
      
      
      
}; // CONTRACT gamedemodemo      




extern "C"
{

void apply(uint64_t receiver, uint64_t code, uint64_t action) 
{

    if (
       (code == name("atomicassets").value) &&
       action == name("transfer").value
       )
       {       
       execute_action(name(receiver), name(code), &gamedemodemo::hdlnft);  
       }       

    if (action == name("transfer").value)
       {       
       //  execute_action(name(receiver), name(code), &gamedemodemo::hdltransfer);  
       }

    if (action == name("status").value)
       {
       execute_action(name(receiver), name(code), &gamedemodemo::status);  
       }

     if (action == name("claim").value)
       {
       execute_action(name(receiver), name(code), &gamedemodemo::claim);  
       }
       
    if (action == name("unstake").value)
       {
       execute_action(name(receiver), name(code), &gamedemodemo::unstake);  
       }

    if (action == name("admin").value)
       {
       execute_action(name(receiver), name(code), &gamedemodemo::admin);  
       }
 


} // apply
    
    
    
       
} // extern "C"      

