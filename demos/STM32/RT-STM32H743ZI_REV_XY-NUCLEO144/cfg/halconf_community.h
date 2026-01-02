#ifndef HALCONF_COMMUNITY_H
#define HALCONF_COMMUNITY_H

/*
 * Enable the community overlay and required FMC/SDRAM drivers.
 */
#if !defined(HAL_USE_COMMUNITY)
#define HAL_USE_COMMUNITY           TRUE
#endif

#if !defined(HAL_USE_FSMC)
#define HAL_USE_FSMC                TRUE
#endif

#if !defined(HAL_USE_SDRAM)
#define HAL_USE_SDRAM               TRUE
#endif

#endif /* HALCONF_COMMUNITY_H */
