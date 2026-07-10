interface Props {
  on: boolean
  onToggle?: () => void
}

/** Sliding toggle — the handoff's toggleOn/toggleKnob, with an off state added. */
export function Toggle({ on, onToggle }: Props) {
  return (
    <div
      onClick={onToggle}
      style={{
        width: 30, height: 15, borderRadius: 8, position: 'relative', flex: 'none',
        background: on ? 'var(--keyA)' : 'var(--inset)',
        border: on ? 'none' : '1px solid var(--panel-b)',
        boxShadow: 'inset 0 1px 2px rgba(0,0,0,.3)',
        cursor: onToggle ? 'pointer' : 'default',
      }}
    >
      <div style={{ position: 'absolute', top: 1, [on ? 'right' : 'left']: 1, width: 13, height: 13, borderRadius: '50%', background: '#f2ecd9', boxShadow: '0 1px 2px rgba(0,0,0,.4)' }} />
    </div>
  )
}
