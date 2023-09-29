/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 13:54:45 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/11 13:22:35 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	unsigned int	i;
	char			*newstring;

	i = 0;
	newstring = (char *)ft_calloc(ft_strlen(s) + 1, sizeof(char));
	if (!newstring)
		return (NULL);
	while (s[i])
	{
		newstring[i] = (*f)(i, s[i]);
		i++;
	}
	newstring[ft_strlen(s)] = 0;
	return (newstring);
}
