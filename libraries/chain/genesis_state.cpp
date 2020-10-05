#include <eosio/chain/genesis_state.hpp>

namespace eosio { namespace chain {

genesis_state_v0::genesis_state_v0() {
   initial_timestamp = fc::time_point::from_iso_string( "2018-06-01T12:00:00" );
   initial_key = fc::variant(genesis_state::eosio_root_key).as<public_key_type>();
}

genesis_state_v1::genesis_state_v1() {
   initial_timestamp = fc::time_point::from_iso_string( "2018-06-01T12:00:00" );
   initial_key = fc::variant(genesis_state::eosio_root_key).as<public_key_type>();
}

genesis_state::genesis_state() : genesis_state(genesis_state_v0{}) {}

genesis_state::genesis_state(genesis_state_v0&& state) : _impl(std::move(state)) {}
genesis_state::genesis_state(genesis_state_v1&& state) : _impl(std::move(state)) {}

chain::chain_id_type genesis_state::compute_chain_id() const {
   digest_type::encoder enc;

   if( const auto* v0 = std::get_if<genesis_state_v0>(&_impl) ) {
      // Preserve original chain_id calculation for v0 genesis state
      fc::raw::pack( enc, *v0 );
   } else {
      fc::raw::pack( enc, *this );
   }

   return chain_id_type{enc.result()};
}

void from_variant(const fc::variant& v, genesis_state& gs) {
   auto obj = v.get_object();
   uint64_t version = 0;
   if(auto iter = obj.find("version"); iter != obj.end()) {
      version = iter->value().as_uint64();
   }
   if(version == 0) {
      gs._impl.emplace<genesis_state_v0>();
      from_variant(v, std::get<genesis_state_v0>(gs._impl));
   } else if(version == 1) {
      gs._impl.emplace<genesis_state_v1>();
      from_variant(v, std::get<genesis_state_v1>(gs._impl));
   } else {
      EOS_THROW(chain_type_exception, "Unknown genesis file version");
   }
}

void to_variant(const genesis_state& gs, fc::variant& v) {
   auto obj = v.get_object();
   fc::mutable_variant_object mvo;
   mvo("version", gs._impl.index());
   std::visit([&](const auto& v) {
      fc::reflector<std::decay_t<decltype(v)>>::visit( fc::to_variant_visitor( mvo, v ) );
   }, gs._impl);
}

} } // namespace eosio::chain
