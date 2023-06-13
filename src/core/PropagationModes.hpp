#ifndef PROPAGATIONMODES_HPP
#define PROPAGATIONMODES_HPP

enum PropagationMode {
  NONE = 0,
  TRANS_SYSTEM_DEFAULT = 1 << 0,
  TRANS_LANGEVIN = 1 << 1,
  TRANS_VS_RELATIVE = 1 << 2,
  TRANS_LB_MOMENTUM_EXCHANGE = 1 << 3,
  TRANS_LB_TRACER = 1 << 4,
  TRANS_BROWNIAN = 1 << 5,
  TRANS_STOKESIAN = 1 << 6,
  ROT_LANGEVIN = 1 << 7,
  ROT_VS_RELATIVE = 1 << 8,
  ROT_BROWNIAN = 1 << 9
};

static bool is_valid_propagation_combination(int propagation) {
  if (propagation == 0)
    return true; // make sure if true or false
  if (!(propagation & (propagation - 1)))
    return true; // only one trans or rot
  if (propagation == TRANS_LANGEVIN + ROT_LANGEVIN ||
      propagation == TRANS_VS_RELATIVE + ROT_VS_RELATIVE ||
      propagation == TRANS_BROWNIAN + ROT_BROWNIAN)
    return true; // same mode for translation and rotation
  if (propagation == TRANS_VS_RELATIVE + ROT_LANGEVIN ||
      propagation == TRANS_LANGEVIN + ROT_VS_RELATIVE)
    return true;
  if (propagation == TRANS_LB_MOMENTUM_EXCHANGE + TRANS_VS_RELATIVE)
    return true;
  if (propagation ==
          TRANS_LB_MOMENTUM_EXCHANGE + TRANS_VS_RELATIVE + ROT_LANGEVIN ||
      propagation ==
          TRANS_LB_MOMENTUM_EXCHANGE + TRANS_VS_RELATIVE + ROT_VS_RELATIVE)
    return true;
  return false;
}

#endif