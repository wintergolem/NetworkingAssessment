using UnityEngine;
using System.Collections;

public class BaseStats
{
	public float health;
	public float attackDamage;
	public float ablityPower;
	public float armor;
	public float magicResist;
	public float attackSpeed;
	public float movementSpeed;
	public float attackRange;

	public static BaseStats operator + (BaseStats b, BaseStats c)
	{
		BaseStats a = new BaseStats();
		a.health = b.health + b.health;
		a.attackDamage = b.attackDamage + c.attackDamage;
		a.ablityPower = b.ablityPower + c.ablityPower;
		a.armor = b.armor + c.armor;
		a.magicResist = b.magicResist + c.magicResist;
		a.attackSpeed = b.attackSpeed + c.attackSpeed;
		a.movementSpeed = b.movementSpeed + c.movementSpeed;
		a.attackRange = b.attackRange + c.attackRange;
		return a;
	}
	public static BaseStats operator - (BaseStats b, BaseStats c)
	{
		BaseStats a = new BaseStats();
		a.health = b.health - b.health;
		a.attackDamage = b.attackDamage - c.attackDamage;
		a.ablityPower = b.ablityPower - c.ablityPower;
		a.armor = b.armor - c.armor;
		a.magicResist = b.magicResist - c.magicResist;
		a.attackSpeed = b.attackSpeed - c.attackSpeed;
		a.movementSpeed = b.movementSpeed - c.movementSpeed;
		a.attackRange = b.attackRange - c.attackRange;
		return a;
	}
}